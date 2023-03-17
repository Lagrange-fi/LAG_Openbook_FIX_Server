#include "SerumMD.h"

#define SERUM_DEBUG

using namespace std;
using namespace std::chrono;
using namespace SerumAdapter;
using namespace BrokerModels;
using namespace marketlib;

void SerumMD::onOpen() {
#ifdef SERUM_DEBUG
_logger->Debug("> SerumMD::onOpen");
#endif
	_onEvent(getName(), "", BrokerEvent::SessionLogon, "Serum DEX Logon: " + getName());
}
void SerumMD::onClose() {
#ifdef SERUM_DEBUG
	_logger->Debug("> SerumMD::onClose");
#endif
	_onEvent(getName(), "", BrokerEvent::SessionLogout, "Serum DEX Logout: " + getName());
	clearMarkets();
}
void SerumMD::onFail() {
#ifdef SERUM_DEBUG
_logger->Debug("> SerumMD::onFail");
#endif
	_onEvent(getName(), "", BrokerEvent::SessionLogout, "Serum DEX Logout: " + getName());
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
	else if (type == "error") {
		_logger->Error(message.c_str());
		if (message.find("Invalid market name provided") != string::npos) {
			auto s1 = message.find("'")+1;
			auto market = message.substr( s1, message.find("'", s1) - s1);

			// broadcastForMarketSubscribers(
			// 	market, 
			// 	SubscriptionModel::top, 
			// 	(boost::format(R"(Subscription to %1% is not supported by %2%)") % market % getName()).str(),
			// 	BrokerEvent::SubscribedCoinIsNotValid
			// );

			// broadcastForMarketSubscribers(
			// 	market, 
			// 	SubscriptionModel::full, 
			// 	(boost::format(R"(Subscription to %1% is not supported by %2%)") % market % getName()).str(),
			// 	BrokerEvent::SubscribedCoinIsNotValid
			// );

			auto it_chnl = _channels
				.get<SubscribeChannelsByMarket>()
				.find(market);
			while (it_chnl != _channels.get<SubscribeChannelsByMarket>().end() && it_chnl->market == market) {
				it_chnl->callback(
					getName(),
					market,
					(boost::format(R"(Subscription to %1% is not supported by %2%)") % market % getName()).str(),
					BrokerEvent::SubscribedCoinIsNotValid
				);
				it_chnl = _channels.get<SubscribeChannelsByMarket>().erase(it_chnl);
			}
		}
		return;
	}

	// logger->Info(message.c_str());
	string market = parsed_data.at("market").as_string().c_str();
	if (type == "quote") {
			bool is_subscribe_quote = false;
			if (!_top_snapshot.count(market))
				is_subscribe_quote = true;
			_top_snapshot[market] = MarketBook{
				system_clock::now(), 
				stod(parsed_data.at("bestBid").at(0).as_string().c_str()),
				stod(parsed_data.at("bestBid").at(1).as_string().c_str()),
				stod(parsed_data.at("bestAsk").at(0).as_string().c_str()),
				stod(parsed_data.at("bestAsk").at(1).as_string().c_str())
			};
		
		broadcastForMarketSubscribers(market, SubscriptionModel::top, _top_snapshot[market], is_subscribe_quote ? BrokerEvent::CoinSubscribed : BrokerEvent::CoinUpdate);
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

		broadcastForMarketSubscribers(market, SubscriptionModel::full, depth, BrokerEvent::CoinSubscribed);
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
			else
				list.sort([](const MarketUpdate&a, const MarketUpdate&b){ return a.price > b.price; });
		};
		string key =  parsed_data.at("market").as_string().c_str();
		auto& depth = _depth_snapshot[key];
		updateDepth(parsed_data.at("asks"), depth.asks, true);
		updateDepth(parsed_data.at("bids"), depth.bids, false);
		broadcastForMarketSubscribers(market, SubscriptionModel::full, depth, BrokerEvent::CoinUpdate);
	}
}

void SerumMD::broadcastForMarketSubscribers(const string& market, SubscriptionModel model, const std::any& data, BrokerEvent event) const {
	auto chnls = _channels
		.get<SubscribeChannelsByMarketAndSubscribeModel>()
		.equal_range(boost::make_tuple(
			market, 
			model
		));
	while(chnls.first != chnls.second){
		chnls.first->callback(
			getName(),
			chnls.first->instr.symbol,
			data,
			event
		);
		++chnls.first;
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
					_settings->get(ISettings::Property::ExchangeName),
					getMarketFromInstrument(instr),
					_top_snapshot[getMarketFromInstrument(instr)],
					BrokerEvent::CoinSubscribed
				);
			}
			else {
				callback(
					_settings->get(ISettings::Property::ExchangeName),
					getMarketFromInstrument(instr),
					_depth_snapshot[getMarketFromInstrument(instr)],
					BrokerEvent::CoinSubscribed
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

	chnl->callback(getName(), getMarketFromInstrument(instr), "", IBrokerClient::BrokerEvent::CoinUnsubscribed);

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
		.equal_range(clientId);

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