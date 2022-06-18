#include "SerumMD.h"
#include "SerumAdapter.h"
#include <fstream>
#include <iterator>
#include <algorithm>
#include <marketlib/include/BrokerModels.h>

#define SERUM_DEBUG

using namespace std;
using namespace std::chrono;
using namespace SerumAdapter;
using namespace BrokerModels; 

void SerumMD::onOpen() {
#ifdef SERUM_DEBUG
logger->Debug("> SerumMD::onOpen");
#endif
	// application->onEvent(getName(), BrokerEvent::SessionLogon, "Serum DEX Logon: " + getName());
}
void SerumMD::onClose() {
#ifdef SERUM_DEBUG
	logger->Debug("> SerumMD::onClose");
#endif
	// application->onEvent(getName(), BrokerEvent::SessionLogout, "Serum DEX Logout: " + getName());
	clearMarkets();
}
void SerumMD::onFail() {
#ifdef SERUM_DEBUG
logger->Debug("> SerumMD::onFail");
#endif
	// application->onEvent(getName(), BrokerEvent::SessionLogout, "Serum DEX Logout: " + getName());
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
		logger->Info(message.c_str());
		return;
	} 

	// logger->Info(message.c_str());
	string market = parsed_data.at("market").as_string().c_str();
	if (type == "quote") {
		auto chnls = channels
			.get<SubscribeChannelsByMarketAndSubscribeModel>()
			.equal_range(boost::make_tuple(
				market, 
				SubscriptionModel::top
			));
		while(chnls.first != chnls.second) {
			chnls.first->callback(
				name,
				market,
				MarketBook{
					system_clock::now(), 
					stod(parsed_data.at("bestBid").at(0).as_string().c_str()),
					stod(parsed_data.at("bestBid").at(1).as_string().c_str()),
					stod(parsed_data.at("bestAsk").at(0).as_string().c_str()),
					stod(parsed_data.at("bestAsk").at(1).as_string().c_str())
				}
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
		depth_snapshot[key] = DepthSnapshot{};
		auto& depth = depth_snapshot[key];
		depth.bids = std::list<BrokerModels::MarketUpdate>();
		depth.asks = std::list<BrokerModels::MarketUpdate>();
		jsonToObject(parsed_data.at("asks"), depth.asks);
		jsonToObject(parsed_data.at("bids"), depth.bids);
		// application->onReport(name, key, depth);

		auto chnls = channels
			.get<SubscribeChannelsByMarketAndSubscribeModel>()
			.equal_range(boost::make_tuple(
				market, 
				SubscriptionModel::full
			));
		while(chnls.first != chnls.second){
			chnls.first->callback(
				name,
				market,
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
		auto& depth = depth_snapshot[key];
		updateDepth(parsed_data.at("asks"), depth.asks, true);
		updateDepth(parsed_data.at("bids"), depth.bids, false);

		auto chnls = channels
			.get<SubscribeChannelsByMarketAndSubscribeModel>()
			.equal_range(boost::make_tuple(
				market, 
				SubscriptionModel::full
			));
		while(chnls.first != chnls.second){
			chnls.first->callback(
				name,
				market,
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
		logger->Warn("Attempt to request disabled client");
	}
	return isEnabled();
}

bool SerumMD::connectedCheck() const {
	if (!isConnected()) {
		logger->Warn("Attempt to request disconnected client");
	}
	return isConnected();
}

bool SerumMD::activeCheck() const {
	return enabledCheck() && connectedCheck();
}

SerumMD::SerumMD(logger_ptr _logger, settings_ptr _settings, pools_ptr pools_):
	logger(_logger), connection(this, _settings->get(ISettings::Property::WebsocketEndpoint), _logger), 
	depth_snapshot(depth_snapshots()), settings(_settings), pools(pools_) {
		pools->loadPools();
	}

bool SerumMD::isEnabled() const {
	return connection.enabled;
}

bool SerumMD::isConnected() const {
	return connection.connected;
}

void SerumMD::clearMarkets() {
#ifdef SERUM_DEBUG
	logger->Debug("> SerumMD::clearMarkets");
#endif
	depth_snapshot.clear();
}

void SerumMD::start() {
	connection.async_start();
}
void SerumMD::stop() {
	connection.async_stop();
}

void SerumMD::subscribe(const instrument& instr, SubscriptionModel model, const string& clientId, callback_t callback) {
	auto chnls = channels
		.get<SubscribeChannelsByMarketAndSubscribeModel>()
		.equal_range(boost::make_tuple(
			getMarketFromInstrument(instr), 
			model
		));
	if (chnls.first == chnls.second) {
		connection.async_send((boost::format(R"({
			"op": "subscribe",
			"channel": "%1%",
			"markets": ["%2%"]
		})") % subscriptionModelToString(model) % getMarketFromInstrument(instr)).str());
	}
	channels.insert(
		SubscribeChannel{
			clientId,
			getMarketFromInstrument(instr),
			model,
			callback
		}
	);
}

void SerumMD::unsubscribe(const instrument& instr, SubscriptionModel model, const string& clientId) {
	auto chnl = channels
		.get<SubscribeChannelsByClientAndMarketAndSubscribeModel>()
		.find(boost::make_tuple(
			clientId,
			getMarketFromInstrument(instr), 
			model
		));

	if (chnl == channels.end()) {
		logger->Error("Subscription not found");
		return;
	}

	channels.erase(chnl);
	auto chnls = channels
		.get<SubscribeChannelsByMarketAndSubscribeModel>()
		.equal_range(boost::make_tuple(
			getMarketFromInstrument(instr), 
			model
		));
	if (chnls.first == chnls.second) {
		connection.async_send((boost::format(R"({
			"op": "unsubscribe",
			"channel": "%1%",
			"markets": ["%2%"]
		})") % subscriptionModelToString(model) % getMarketFromInstrument(instr)).str());
	}
}

static size_t writeCallback(void* content, size_t size, size_t count, void* result) {
	((string*)result)->append((char*)content, size * count);
	return size * count;
}

std::vector< SerumMD::instrument > SerumMD::getInstruments() {
    return pools->getPools();
}


string SerumMD::getName() const {
	return settings->get(ISettings::Property::ExchangeName);
}

SerumMD::~SerumMD() {
	connection.async_stop();
	while (isConnected()) continue;
}