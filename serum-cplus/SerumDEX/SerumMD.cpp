#include "SerumMD.h"

#define SERUM_DEBUG
#define DATA_CHANNEL_CAST_TO_TOP_DATA(x) ((TopDataChannel*)&(*x))
#define DATA_CHANNEL_CAST_TO_DEPTH_DATA(x) ((DepthDataChannel*)&(*x))

using namespace std;
using namespace std::chrono;
using namespace SerumAdapter;
using namespace BrokerModels;
using namespace marketlib;

void SerumMD::onOpen() {
#ifdef SERUM_DEBUG
_logger->Debug("> SerumMD::onOpen");
#endif
	_onEvent(getName(), broker_event::session_logon, "Serum DEX Logon: " + getName());
}
void SerumMD::onClose() {
#ifdef SERUM_DEBUG
	_logger->Debug("> SerumMD::onClose");
#endif
	_onEvent(getName(), broker_event::session_logout, "Serum DEX Logout: " + getName());
	clearMarkets();
}
void SerumMD::onFail() {
#ifdef SERUM_DEBUG
_logger->Debug("> SerumMD::onFail");
#endif
	_onEvent(getName(), broker_event::session_logout, "Serum DEX Logout: " + getName());
	clearMarkets();
}
void SerumMD::onMessage(const string& message) {
	if (message[0] == '{') {
		onEventHandler(message);
	} else {
		onUpdateHandler(message);
	}
}

void SerumMD::onEventHandler(const string &message) {
	// logger->Info(message.c_str());

	auto parsed_data = boost::json::parse(message);

	string type = parsed_data.at("type").as_string().c_str();
	if (type == "subscribed" || type == "unsubscribed") {
		_logger->Info(message.c_str());
		return;
	} 

	// logger->Info(message.c_str());
	string market = parsed_data.at("market").as_string().c_str();
	if (type == "quote") {
			_top_snapshot[market] = MarketBook{
				system_clock::now(), 
				stod(parsed_data.at("bestBid").at(0).as_string().c_str()),
				stod(parsed_data.at("bestBid").at(1).as_string().c_str()),
				stod(parsed_data.at("bestAsk").at(0).as_string().c_str()),
				stod(parsed_data.at("bestAsk").at(1).as_string().c_str())
			};
		
		auto chnls = _channels
			.get<SubscribeChannelsByMarketAndSubscribeModel>()
			.equal_range(boost::make_tuple(
				market, 
				SubscriptionModel::top
			));
		while(chnls.first != chnls.second) {
			chnls.first->callback(
				_name,
				chnls.first->instr.symbol,
				_top_snapshot[market]
			);
			++chnls.first;
  		}
	} else if (type == "l2snapshot") {
		auto jsonToObject = [](const boost::json::value& val, std::list<BrokerModels::MarketUpdate>& vec) {
			for(auto set : val.as_array()) {
  				vec.push_back(MarketUpdate{
					stod(set.at(0).as_string().c_str()),
					stod(set.at(1).as_string().c_str())
				});
			}
		};
		string key =  parsed_data.at("market").as_string().c_str();
		_depth_snapshot[key] = DepthSnapshot{};
		auto& depth = _depth_snapshot[key];
		depth.bids = std::list<BrokerModels::MarketUpdate>();
		depth.asks = std::list<BrokerModels::MarketUpdate>();
		jsonToObject(parsed_data.at("asks"), depth.asks);
		jsonToObject(parsed_data.at("bids"), depth.bids);
		// application->onReport(name, key, depth);

		auto chnls = _channels
			.get<SubscribeChannelsByMarketAndSubscribeModel>()
			.equal_range(boost::make_tuple(
				market, 
				SubscriptionModel::full
			));
		while(chnls.first != chnls.second){
			chnls.first->callback(
				_name,
				chnls.first->instr.symbol,
				depth
			);
			++chnls.first;
  		}
	} else if (type == "l2update") {
		// logger->Info(message.c_str());
		auto updateDepth = [](const boost::json::value& val, std::list<BrokerModels::MarketUpdate>& list, bool is_ask) {
			for(auto set : val.as_array()) {
  				if (stod(set.at(1).as_string().c_str()) == 0.) {
					list.erase(std::find_if(
						begin(list), 
						end(list), 
						[price = stod(set.at(0).as_string().c_str())](MarketUpdate market){ return market.price==price; }
					));
				}
				else {
					list.push_back(MarketUpdate{
						stod(set.at(0).as_string().c_str()),
						stod(set.at(1).as_string().c_str())
					});
				}
			}

			if (is_ask)
				list.sort([](const MarketUpdate&a, const MarketUpdate&b){ return a.price < b.price; });
				// std::sort(begin(vec), end(vec), 
				// 	[](const MarketUpdate&a, const MarketUpdate&b){ return a.price < b.price; });
			else
				list.sort([](const MarketUpdate&a, const MarketUpdate&b){ return a.price > b.price; });
				// std::sort(begin(vec), end(vec), 
				// 	[](const MarketUpdate&a, const MarketUpdate&b){ return a.price > b.price; });
		};
		string key =  parsed_data.at("market").as_string().c_str();
		auto& depth = _depth_snapshot[key];
		updateDepth(parsed_data.at("asks"), depth.asks, true);
		updateDepth(parsed_data.at("bids"), depth.bids, false);

		auto chnls = _channels
			.get<SubscribeChannelsByMarketAndSubscribeModel>()
			.equal_range(boost::make_tuple(
				market, 
				SubscriptionModel::full
			));
		while(chnls.first != chnls.second){
			chnls.first->callback(
				_name,
				chnls.first->instr.symbol,
				depth
			);
			++chnls.first;
  		}
	}
}

void SerumMD::onUpdateHandler(const string &message) {
	
}

bool SerumMD::enabledCheck() const {
	if (!isEnabled()) {
		_logger->Warn("Attempt to request disabled client");
	}
	return isEnabled();
}

bool SerumMD::connectedCheck() const {
	if (!isConnected()) {
		_logger->Warn("Attempt to request disconnected client");
	}
	return isConnected();
}

bool SerumMD::activeCheck() const {
	return enabledCheck() && connectedCheck();
}

SerumMD::SerumMD(logger_ptr logger, settings_ptr settings, pools_ptr pools, callback_on_event OnEvent):
	_logger(logger), _connection(this, settings->get(ISettings::Property::WebsocketEndpoint), logger), 
	_depth_snapshot(depth_snapshots()), _settings(settings), _pools(pools), _onEvent(OnEvent) {
		// pools->loadPools();
	}

bool SerumMD::isEnabled() const {
	return _connection.enabled;
}

bool SerumMD::isConnected() const {
	return _connection.connected;
}

void SerumMD::clearMarkets() {
#ifdef SERUM_DEBUG
	_logger->Debug("> SerumMD::clearMarkets");
#endif
	_depth_snapshot.clear();
	_top_snapshot.clear();
	_channels.clear();
}

void SerumMD::start() {
	_connection.async_start();
}
void SerumMD::stop() {
	_connection.async_stop();
	clearMarkets();
}

void SerumMD::subscribe(const instrument& instr, SubscriptionModel model) {
	_connection.async_send((boost::format(R"({
		"op": "subscribe",
		"channel": "%1%",
		"markets": ["%2%"]
	})") % subscriptionModelToString(model) % getMarketFromInstrument(instr)).str());
	
}

void SerumMD::subscribe(const instrument& instr, SubscriptionModel model, const string& clientId, callback_subscribed_channel callback) {
	auto chnls = _channels
		.get<SubscribeChannelsByMarketAndSubscribeModel>()
		.equal_range(boost::make_tuple(
			getMarketFromInstrument(instr), 
			model
		));
	
	if (chnls.first == chnls.second)
		subscribe(instr, model);

	auto chnl_by_client = _channels
		.get<SubscribeChannelsByClientAndMarketAndSubscribeModel>()
		.find(boost::make_tuple(
			clientId,
			getMarketFromInstrument(instr), 
			model
		));

	if (chnl_by_client == _channels.end()) {
		if (chnls.first != chnls.second)
			if (model == SubscriptionModel::top) {
				callback(
					_name,
					getMarketFromInstrument(instr),
					_top_snapshot[getMarketFromInstrument(instr)]
				);
			}
			else {
				callback(
					_name,
					getMarketFromInstrument(instr),
					_depth_snapshot[getMarketFromInstrument(instr)]
				);
			}
				
		_channels.insert(
			SubscribeChannel{
				clientId: clientId,
				market: getMarketFromInstrument(instr),
				instr: instr,
				smodel: model,
				callback: callback,
			}
		); 
	}
	else {
		string msg = "The subscription with parameters already exists";
		msg += ": symbol - " + getMarketFromInstrument(instr);
		msg += ", client Id - " + clientId;
		msg += ", subscription model - ";
		msg += model == SubscriptionModel::top ? "top" : "full";

		_logger->Error( msg.c_str() );
	}
}

// void SerumMD::subscribe(const instrument& instr, const string& clientId, callbackTop callback) {
// 	auto chnls = channels
// 		.get<SubscribeChannelsByMarketAndSubscribeModel>()
// 		.equal_range(boost::make_tuple(
// 			getMarketFromInstrument(instr), 
// 			SubscriptionModel::top
// 		));
	
// 	if (chnls.first == chnls.second) {
// 		subscribe(instr, SubscriptionModel::top);
// 	} else {
// 		callback(
// 			name,
// 			instr,
// 			top_snapshot[getMarketFromInstrument(instr)]
// 		);
// 	}

	
// 	channels.insert(
// 		SubscribeChannel{
// 			clientId: clientId,
// 			market: getMarketFromInstrument(instr),
// 			instr: instr,
// 			smodel: SubscriptionModel::top,
// 			callback_top: callback,
// 			callback_depth: nullptr
// 		}
// 	); 
// }

// void SerumMD::subscribe(const instrument& instr, const string& clientId, callbackDepth callback) {
// 	auto chnls = channels
// 		.get<SubscribeChannelsByMarketAndSubscribeModel>()
// 		.equal_range(boost::make_tuple(
// 			getMarketFromInstrument(instr), 
// 			SubscriptionModel::full
// 		));
	
// 	if (chnls.first == chnls.second) {
// 		subscribe(instr, SubscriptionModel::full);
// 	} else {
// 		callback(
// 			name,
// 			instr,
// 			depth_snapshot[getMarketFromInstrument(instr)]
// 		);
// 	}

// 	channels.insert(
// 		SubscribeChannel{
// 			clientId: clientId,
// 			market: getMarketFromInstrument(instr),
// 			instr: instr,
// 			smodel: SubscriptionModel::full,
// 			callback_top: nullptr,
// 			callback_depth: callback
// 		}
// 	); 
// }

void SerumMD::unsubscribe(const instrument& instr, SubscriptionModel model, const string& clientId) {
	auto chnl = _channels
		.get<SubscribeChannelsByClientAndMarketAndSubscribeModel>()
		.find(boost::make_tuple(
			clientId,
			getMarketFromInstrument(instr), 
			model
		));

	if (chnl == _channels.end()) {
		_logger->Error("Subscription not found");
		return;
	}

	_channels.erase(chnl);
	auto chnls = _channels
		.get<SubscribeChannelsByMarketAndSubscribeModel>()
		.equal_range(boost::make_tuple(
			getMarketFromInstrument(instr), 
			model
		));
	if (chnls.first == chnls.second) {
		_connection.async_send((boost::format(R"({
			"op": "unsubscribe",
			"channel": "%1%",
			"markets": ["%2%"]
		})") % subscriptionModelToString(model) % getMarketFromInstrument(instr)).str());

		if (model == SubscriptionModel::top) {
			_top_snapshot.erase(getMarketFromInstrument(instr));
		} else {
			_depth_snapshot.erase(getMarketFromInstrument(instr));
		}
	}
}

void SerumMD::unsubscribeForClientId(const string& clientId) {
	auto chnls = _channels
		.get<SubscribeChannelsByClient>()
		.equal_range(boost::make_tuple(
			clientId
		));

	std::list<std::pair<instrument, SubscriptionModel>> info;
	while(chnls.first != chnls.second) {
		info.push_back(std::pair<instrument, SubscriptionModel>
			(chnls.first->instr, chnls.first->smodel));
		++chnls.first;
	}

	for (const auto& a : info)
		unsubscribe(a.first, a.second, clientId);
}

static size_t writeCallback(void* content, size_t size, size_t count, void* result) {
	((string*)result)->append((char*)content, size * count);
	return size * count;
}

std::list< SerumMD::instrument > SerumMD::getInstruments() {
    return _pools->getPools();
}


string SerumMD::getName() const {
	return _settings->get(ISettings::Property::ExchangeName);
}

SerumMD::~SerumMD() {
	_connection.async_stop();
	while (isConnected()) continue;
}