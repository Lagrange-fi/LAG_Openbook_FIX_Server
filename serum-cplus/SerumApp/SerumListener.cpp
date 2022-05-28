#include "SerumListener.h"
#include "SerumAdapter.h"
#include <fstream>

#include "BrokerLib/BrokerModels.h"

// #define SERUM_LISTENER_DEBUG

using namespace std;
using namespace std::chrono;
using namespace SerumAdapter;
using namespace BrokerModels; 

void SerumListener::onOpen() {
#ifdef SERUM_LISTENER_DEBUG
logger->Debug("> SerumListener::onOpen");
#endif
	// application->onEvent(getName(), BrokerEvent::SessionLogon, "Logon: " + getName());
}
void SerumListener::onClose() {
#ifdef SERUM_LISTENER_DEBUG
	logger->Debug("> SerumListener::onClose");
#endif
	// application->onEvent(getName(), BrokerEvent::SessionLogout, "Logon: " + getName());
	// clearMarkets();
}
void SerumListener::onFail() {
#ifdef SERUM_LISTENER_DEBUG
logger->Debug("> SerumListener::onFail");
#endif
	// application->onEvent(getName(), BrokerEvent::SessionLogout, "Logon: " + getName());
	// clearMarkets();
}
void SerumListener::onMessage(const string& message) {
	if (message.find("filled")!= std::string::npos) {
		auto tt = 0;
	}
	if (message[0] == '{') {
		onEventHandler(message);
	} else { 
		onUpdateHandler(message);
	}
}

void SerumListener::onEventHandler(const string &message) {
	auto parsed_data = boost::json::parse(message);
	std::string type = parsed_data.at("type").as_string().c_str();
#ifdef SERUM_LISTENER_DEBUG
if (type == "subscribed" || type == "unsubscribed") {
	logger->Debug(message.c_str());
} 
#endif
	if (type == "subscribed" || type == "unsubscribed")
		return;
	std::string market = parsed_data.at("market").as_string().c_str();
	if (type  == "l3snapshot" || type  == "open") {
		std::string timestamp = parsed_data.at("timestamp").as_string().c_str();
		auto ind = market.find('/');
		std::string first = market.substr(0,ind);
		std::string second = market.substr(ind + 1);
		auto addOrderToList = [&](const boost::json::value& set, std::list<BrokerModels::Order>& vec) {
			vec.push_back(Order{
				OrderType::Limit,
				OrderState::Open,
				stringToOrderSide(set.at("side").as_string().c_str()),
				stod(set.at("size").as_string().c_str()),
				stod(set.at("price").as_string().c_str()),
				set.at("clientId").as_string().c_str(),
				set.at("orderId").as_string().c_str(),
				timestamp,
				this->settings->get(ISettings::Property::ExchangeName),
				market,
				first,
				second
			});
		};

		if (type  == "l3snapshot") {
			orders[market] = std::list<BrokerModels::Order>{};
			auto& orders_list = orders[market];
			for (const auto &set : parsed_data.at("asks").as_array()) {
				addOrderToList(set, orders_list);
			};
			for (auto set : parsed_data.at("bids").as_array()) {
				addOrderToList(set, orders_list);
			};
		}
		else {
			addOrderToList(parsed_data, orders[market]);
		}
		
	} else if (type  == "change") {
		auto& orders_lst = orders[market];
		auto order = find_if(orders_lst.begin(), orders_lst.end(), [id = parsed_data.at("orderId").as_string().c_str()](auto a) {
			return a.exchangeId == id;
		});
		order->orderSide = stringToOrderSide(parsed_data.at("side").as_string().c_str());
		order->amount = stod(parsed_data.at("size").as_string().c_str());
		order->price = stod(parsed_data.at("price").as_string().c_str());
		order->clientId = parsed_data.at("clientId").as_string().c_str();
		order->initDate = parsed_data.at("timestamp").as_string().c_str();

		application->onReport(settings->get(ISettings::Property::ExchangeName), ExecutionReport{
			order->clientId,
			order->exchangeId,
			order->symbol,
			order->orderSide,
			OrderState::Replaced,
			order->orderType,
			order->price,
			0.,
			order->amount
		});
	}
	else if (type == "done") {
		auto& orders_lst = orders[market];
		auto order = find_if(orders_lst.begin(), orders_lst.end(), [id = parsed_data.at("orderId").as_string().c_str()](auto a) {
			return a.exchangeId == id;
		});
		order = find_if(
			orders_lst.begin(), 
			orders_lst.end(), 
			[id = parsed_data.at("orderId").as_string().c_str()](auto a) {
				return a.exchangeId == id;
			}
		);

		/*"done" can be pushed for orders that were never open in the order book 
		in the first place (ImmediateOrCancel orders for example)*/
		bool is_canceled = string(parsed_data.at("reason").as_string().c_str()) == string("canceled");
		if (order == orders_lst.end()) {
			application->onReport(settings->get(ISettings::Property::ExchangeName), ExecutionReport{
				parsed_data.at("clientId").as_string().c_str(),
				parsed_data.at("orderId").as_string().c_str(),
				parsed_data.at("market").as_string().c_str(),
				stringToOrderSide(parsed_data.at("side").as_string().c_str()),
				is_canceled ? OrderState::Canceled : OrderState::Filled,
				OrderType::Market,
				0,
				0,
				0
			});
			return;
		}
		logger->Info(message.c_str());
		double remaining = is_canceled ? stod(parsed_data.at("sizeRemaining").as_string().c_str()) : 0;
		application->onReport(settings->get(ISettings::Property::ExchangeName), ExecutionReport{
			order->clientId,
			order->exchangeId,
			order->symbol,
			order->orderSide,
			is_canceled ? OrderState::Canceled : OrderState::Filled,
			order->orderType,
			order->price,
			order->amount - remaining,
			remaining
		});
		orders_lst.erase(
			find_if(
				orders_lst.begin(), 
				orders_lst.end(), 
				[id = parsed_data.at("orderId").as_string().c_str()](auto a) {
					return a.exchangeId == id;
				}
			)
		);

		auto tt = 0;
	}
}

void SerumListener::onUpdateHandler(const string &message) {
	
}

bool SerumListener::enabledCheck() const {
	if (!isEnabled()) {
		logger->Warn("Attempt to request disabled client");
	}
	return isEnabled();
}

bool SerumListener::connectedCheck() const {
	if (!isConnected()) {
		logger->Warn("Attempt to request disconnected client");
	}
	return isConnected();
}

bool SerumListener::activeCheck() const {
	return enabledCheck() && connectedCheck();
}

SerumListener::SerumListener(logger_ptr _logger, application_ptr application, settings_ptr _settings):
	logger(_logger), application(application), connection(this, _settings->get(ISettings::Property::WebsocketEndpoint), _logger), 
	settings(_settings) {}

bool SerumListener::isEnabled() const {
	return connection.enabled;
}

bool SerumListener::isConnected() const {
	return connection.connected;
}

// void SerumListener::clearMarkets() {
// #ifdef SERUM_LISTENER_DEBUG
// 	logger->debug("> SerumListener::clearMarkets");
// #endif
// 	depth_snapshot.clear();
// }

void SerumListener::start() {
	connection.async_start();
}
void SerumListener::stop() {
	connection.async_stop();
}

void SerumListener::listen(const BrokerModels::Instrument& instr) {
	connection.async_send((boost::format(R"({
			"op": "subscribe",
			"channel": "level3",
			"markets": ["%1%"]
		})") % getMarket(instr)).str());
}

void SerumListener::unlisten(const BrokerModels::Instrument& instr) {
	connection.async_send((boost::format(R"({
			"op": "unsubscribe",
			"channel": "level3",
			"markets": ["%1%"]
		})") % getMarket(instr)).str());
}

SerumListener::~SerumListener() {
	connection.async_stop();
	while (isConnected()) continue;
}