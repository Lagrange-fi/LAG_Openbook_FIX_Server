#include "SerumTrade.h"

#define SERUM_LISTENER_DEBUG

using namespace std;
using namespace std::chrono;
using namespace SerumAdapter;
using namespace BrokerModels;
using namespace marketlib;

void SerumTrade::onOpen() {
#ifdef SERUM_LISTENER_DEBUG
logger_->Debug("> SerumTrade::onOpen");
#endif
	onEvent_(getName(), broker_event::session_logon, "TradeLogon: " + getName());
}
void SerumTrade::onClose() {
#ifdef SERUM_LISTENER_DEBUG
	logger_->Debug("> SerumTrade::onClose");
#endif
	onEvent_(getName(), broker_event::session_logout, "TradeLogout: " + getName());
	clearMarkets();
}
void SerumTrade::onFail() {
#ifdef SERUM_LISTENER_DEBUG
logger_->Debug("> SerumTrade::onFail");
#endif
	onEvent_(getName(), broker_event::session_logout, "TradeLogout: " + getName());
	clearMarkets();
}
void SerumTrade::onMessage(const string& message) {
	if (message.find("filled")!= std::string::npos) {
		auto tt = 0;
	}
	if (message[0] == '{') {
		onEventHandler(message);
	} else { 
		onUpdateHandler(message);
	}
}

void SerumTrade::onEventHandler(const string &message) {
	auto parsed_data = boost::json::parse(message);
	std::string type = parsed_data.at("type").as_string().c_str();
#ifdef SERUM_LISTENER_DEBUG
if (type == "subscribed" || type == "unsubscribed") {
	logger_->Debug(message.c_str());
} 
#endif
	if (type == "subscribed" || type == "unsubscribed")
		return;
	std::string market = parsed_data.at("market").as_string().c_str();
	if (type  == "l3snapshot" || type  == "open") {
		auto addOrderToList = [&](const boost::json::value& set, std::list<Order>& vec) {
			vec.push_back(Order{
				clId:  strtoull(set.at("clientId").as_string().c_str(), nullptr, 0), // (uint64_t)stoul(set.at("clientId").as_string().c_str()),
				exchId: atouint128(set.at("orderId").as_string().c_str()),
				secId: "",
				transaction_hash: "",
				original_qty: stod(set.at("size").as_string().c_str()),
				remaining_qty: stod(set.at("size").as_string().c_str()),
				price: stod(set.at("price").as_string().c_str()),
				stopPrice: 0.0,
				side: stringToOrderSide(set.at("side").as_string().c_str()),
				state: order_state_t::ost_New,
				tif: time_in_force_t::tf_Undefined,
				type: order_type_t::ot_Limit
			});
		};

		if (type  == "l3snapshot") {
			orders_[market] = std::list<Order>{};
			auto& orders_list = orders_[market];
			for (const auto &set : parsed_data.at("asks").as_array()) {
				addOrderToList(set, orders_list);
			};
			for (auto set : parsed_data.at("bids").as_array()) {
				addOrderToList(set, orders_list);
			};
		}
		else {
			addOrderToList(parsed_data, orders_[market]);
		}
		
	} else if (type  == "change") {
		auto& orders_lst = orders_[market];
		auto exch_id = atouint128(parsed_data.at("orderId").as_string().c_str());
		auto order = find_if(orders_lst.begin(), orders_lst.end(), [exch_id](auto a) {
			return a.exchId == exch_id;
		});
		order->side = stringToOrderSide(parsed_data.at("side").as_string().c_str());
		order->original_qty = stod(parsed_data.at("size").as_string().c_str());
		order->remaining_qty = stod(parsed_data.at("size").as_string().c_str());
		order->price = stod(parsed_data.at("price").as_string().c_str());
		order->clId = strtoull(parsed_data.at("clientId").as_string().c_str(), nullptr, 0);

		auto report = ExecutionReport();
		report.clId = order->clId;
		report.exchId = order->exchId;
		report.orderType = order->type;
		report.type = report_type_t::rt_replaced;
		report.state = order->state;
		report.side = order->side;
		report.limitPrice = order->price;
		report.leavesQty = order->original_qty;

		// application->onReport(settings->get(ISettings::Property::ExchangeName), market, std::move(report));
		auto chnls = channels_
			.get<SubscribeChannelsByMarket>()
			.equal_range(boost::make_tuple(market));
		while(chnls.first != chnls.second) {
			chnls.first->callback(
				settings_->get(ISettings::Property::ExchangeName),
				market,
				report
			);
			++chnls.first;
  		}
	}
	else if (type == "done") {
		auto& orders_lst = orders_[market];
		auto exch_id = atouint128(parsed_data.at("orderId").as_string().c_str());
		auto order = find_if(orders_lst.begin(), orders_lst.end(), [exch_id](auto a) {
			return a.exchId == exch_id;
		});
		// order = find_if(
		// 	orders_lst.begin(), 
		// 	orders_lst.end(), 
		// 	[id = parsed_data.at("orderId").as_string().c_str()](auto a) {
		// 		return a.exchId == id;
		// 	}
		// );

		/*"done" can be pushed for orders that were never open in the order book 
		in the first place (ImmediateOrCancel orders for example)*/
		bool is_canceled = string(parsed_data.at("reason").as_string().c_str()) == string("canceled");
		if (order == orders_lst.end()) {
			auto report = ExecutionReport();
			report.clId = strtoull(parsed_data.at("clientId").as_string().c_str(), nullptr, 0);
			report.exchId = exch_id;
			report.orderType = order_type_t::ot_Market;
			report.type = is_canceled ? report_type_t::rt_canceled : report_type_t::rt_fill;
			report.state = is_canceled ? order_state_t::ost_Canceled : order_state_t::ost_Filled;
			report.side = stringToOrderSide(parsed_data.at("side").as_string().c_str());

			auto chnls = channels_
			.get<SubscribeChannelsByMarket>()
			.equal_range(boost::make_tuple(market));
			while(chnls.first != chnls.second) {
				chnls.first->callback(
					settings_->get(ISettings::Property::ExchangeName),
					market,
					report
				);
				++chnls.first;
			}
			return;
		}
		// logger->Info(message.c_str());
		double remaining = is_canceled ? stod(parsed_data.at("sizeRemaining").as_string().c_str()) : 0;
		auto report = ExecutionReport();
		report.clId = order->clId;
		report.exchId = order->exchId;
		report.orderType = order->type;
		report.type = is_canceled ? report_type_t::rt_canceled : report_type_t::rt_fill;
		report.state = is_canceled ? order_state_t::ost_Canceled : order_state_t::ost_Filled;
		report.side = order->side;
		report.limitPrice = order->price;
		report.leavesQty = order->original_qty - remaining;
		report.cumQty = remaining;
		
		auto chnls = channels_
			.get<SubscribeChannelsByMarket>()
			.equal_range(boost::make_tuple(market));
		while(chnls.first != chnls.second) {
			chnls.first->callback(
				settings_->get(ISettings::Property::ExchangeName),
				market,
				report
			);
			++chnls.first;
  		}
		orders_lst.erase(order);
	}
}

void SerumTrade::onUpdateHandler(const string &message) {
	
}

bool SerumTrade::enabledCheck() const {
	if (!isEnabled()) {
		logger_->Warn("Attempt to request disabled client");
	}
	return isEnabled();
}

bool SerumTrade::connectedCheck() const {
	if (!isConnected()) {
		logger_->Warn("Attempt to request disconnected client");
	}
	return isConnected();
}

bool SerumTrade::activeCheck() const {
	return enabledCheck() && connectedCheck();
}

SerumTrade::SerumTrade(logger_ptr logger, settings_ptr settings, callback_on_event OnEvent):
	logger_(logger), connection_(this, settings->get(ISettings::Property::WebsocketEndpoint), logger), 
	settings_(settings), onEvent_(OnEvent) {}

bool SerumTrade::isEnabled() const {
	return connection_.enabled;
}

bool SerumTrade::isConnected() const {
	return connection_.connected;
}

void SerumTrade::clearMarkets() {
#ifdef SERUM_LISTENER_DEBUG
	// logger_->Debug("> SerumTrade::clearMarkets");
#endif
	orders_.clear();
	channels_.clear();
}

void SerumTrade::start() {
	connection_.async_start();
}
void SerumTrade::stop() {
	connection_.async_stop();
	clearMarkets();
}

void SerumTrade::listen(const SerumTrade::Instrument& instr, const string& clientId, callback_t callback) {
	auto chnls = channels_
		.get<SubscribeChannelsByMarket>()
		.equal_range(boost::make_tuple(
			getMarketFromInstrument(instr)
		));

	if (chnls.first == chnls.second) {
		connection_.async_send((boost::format(R"({
			"op": "subscribe",
			"channel": "level3",
			"markets": ["%1%"]
		})") % getMarketFromInstrument(instr)).str());
	}
	channels_.insert(
		SubscribeChannel{
			clientId,
			getMarketFromInstrument(instr),
			instr,
			callback
		}
	);
}

void SerumTrade::unlisten(const SerumTrade::Instrument& instr, const string& clientId) {
	auto chnl = channels_
		.get<SubscribeChannelsByClientAndMarket>()
		.find(boost::make_tuple(
			clientId,
			getMarketFromInstrument(instr)
		));

	if (chnl == channels_.end()) {
		logger_->Error("Subscription not found");
		return;
	}

	channels_.erase(chnl);
	auto chnls = channels_
		.get<SubscribeChannelsByMarket>()
		.equal_range(boost::make_tuple(
			getMarketFromInstrument(instr)
		));
	if (chnls.first == chnls.second) {
		connection_.async_send((boost::format(R"({
			"op": "unsubscribe",
			"channel": "level3",
			"markets": ["%1%"]
		})") % getMarketFromInstrument(instr)).str());
	}
}

void SerumTrade::unlistenForClientId(const string& clientId) {
	auto chnls = channels_
		.get<SubscribeChannelsByClient>()
		.equal_range(boost::make_tuple(
			clientId
		));

	while(chnls.first != chnls.second) {
		unlisten(chnls.first->instr, clientId);
		++chnls.first;
	}
}

string SerumTrade::getName() const {
	return settings_->get(ISettings::Property::ExchangeName);
}

SerumTrade::~SerumTrade() {
	connection_.async_stop();
	while (isConnected()) continue;
}
