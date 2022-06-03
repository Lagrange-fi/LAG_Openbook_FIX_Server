#include "SerumMD.h"
#include "SerumAdapter.h"
#include <fstream>
#include <marketlib/include/BrokerModels.h>

// #define SERUM_DEBUG

using namespace std;
using namespace std::chrono;
using namespace SerumAdapter;
using namespace BrokerModels; 

void SerumMD::onOpen() {
#ifdef SERUM_DEBUG
logger->debug("> SerumMD::onOpen");
#endif
	application->onEvent(getName(), BrokerEvent::SessionLogon, "Logon: " + getName());
}
void SerumMD::onClose() {
#ifdef SERUM_DEBUG
	logger->debug("> SerumMD::onClose");
#endif
	application->onEvent(getName(), BrokerEvent::SessionLogout, "Logon: " + getName());
	clearMarkets();
}
void SerumMD::onFail() {
#ifdef SERUM_DEBUG
logger->debug("> SerumMD::onFail");
#endif
	application->onEvent(getName(), BrokerEvent::SessionLogout, "Logon: " + getName());
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
	if (type == "subscribe" || type == "unsubscribe") {
		logger->Debug(message.c_str());
	} 
	else if (type == "quote") {

		application->onReport(
			name, 
			parsed_data.at("market").as_string().c_str(), 
			MarketBook{
				system_clock::now(), 
				stod(parsed_data.at("bestBid").at(0).as_string().c_str()),
				stod(parsed_data.at("bestBid").at(1).as_string().c_str()),
				stod(parsed_data.at("bestAsk").at(0).as_string().c_str()),
				stod(parsed_data.at("bestAsk").at(1).as_string().c_str())
			});
	} else if (type == "l2snapshot") {
		auto jsonToObject = [](const boost::json::value& val, std::vector<BrokerModels::MarketUpdate>& vec) {
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
		depth.bids = std::vector<BrokerModels::MarketUpdate>();
		depth.asks = std::vector<BrokerModels::MarketUpdate>();
		jsonToObject(parsed_data.at("asks"), depth.asks);
		jsonToObject(parsed_data.at("bids"), depth.bids);
		application->onReport(name, key, depth);
	} else if (type == "l2update") {
		// logger->Info(message.c_str());
		auto updateDepth = [](const boost::json::value& val, std::vector<BrokerModels::MarketUpdate>& vec, bool is_ask) {
			for(auto set : val.as_array()) {
  				if (stod(set.at(1).as_string().c_str()) == 0.) {
					vec.erase(std::find_if(
						begin(vec), 
						end(vec), 
						[price = stod(set.at(0).as_string().c_str())](MarketUpdate market){ return market.price==price; }
					));
				}
				else {
					vec.push_back(MarketUpdate{
						stod(set.at(0).as_string().c_str()),
						stod(set.at(1).as_string().c_str())
					});
				}
			}

			if (is_ask)
				std::sort(begin(vec), end(vec), 
					[](const MarketUpdate&a, const MarketUpdate&b){ return a.price < b.price; });
			else
				std::sort(begin(vec), end(vec), 
					[](const MarketUpdate&a, const MarketUpdate&b){ return a.price > b.price; });
		};
		string key =  parsed_data.at("market").as_string().c_str();
		auto& depth = depth_snapshot[key];
		updateDepth(parsed_data.at("asks"), depth.asks, true);
		updateDepth(parsed_data.at("bids"), depth.bids, false);
		application->onReport(name, key, depth);
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

SerumMD::SerumMD(logger_ptr _logger, IBrokerApplication* application, settings_ptr _settings, pools_ptr pools_):
	logger(_logger), application(application), connection(this, _settings->get(ISettings::Property::WebsocketEndpoint), _logger), 
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
	logger->debug("> SerumMD::clearMarkets");
#endif
	depth_snapshot.clear();
}

void SerumMD::start() {
	connection.async_start();
}
void SerumMD::stop() {
	connection.async_stop();
}

void SerumMD::subscribe(const instrument& instr, SubscriptionModel model) {
	connection.async_send((boost::format(R"({
			"op": "subscribe",
			"channel": "%1%",
			"markets": ["%2%"]
		})") % subscriptionModelToString(model) % instr.symbol).str());
}

void SerumMD::unsubscribe(const instrument& instr, SubscriptionModel model) {
	connection.async_send((boost::format(R"({
			"op": "unsubscribe",
			"channel": "%1%",
			"markets": ["%2%"]
		})") % subscriptionModelToString(model) % instr.symbol).str());
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