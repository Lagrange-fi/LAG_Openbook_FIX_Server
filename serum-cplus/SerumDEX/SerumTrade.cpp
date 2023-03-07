#include "SerumTrade.h"

#define SERUM_LISTENER_DEBUG

using namespace std;
using namespace std::chrono;
using namespace SerumAdapter;
using namespace BrokerModels;
using namespace marketlib;

void SerumTrade::onOpen() {
#ifdef SERUM_LISTENER_DEBUG
_logger->Debug("> SerumTrade::onOpen");
#endif
	_onEvent(getName(), "", broker_event::session_logon, "TradeLogon: " + getName());
}
void SerumTrade::onClose() {
#ifdef SERUM_LISTENER_DEBUG
	_logger->Debug("> SerumTrade::onClose");
#endif
	_onEvent(getName(), "", broker_event::session_logout, "TradeLogout: " + getName());
	clearMarkets();
}
void SerumTrade::onFail() {
#ifdef SERUM_LISTENER_DEBUG
_logger->Debug("> SerumTrade::onFail");
#endif
	_onEvent(getName(), "", broker_event::session_logout, "TradeLogout: " + getName());
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
		_logger->Debug(message.c_str());
	} 
#endif
	if (type == "subscribed" || type == "unsubscribed")
		return;
	else if (type == "error") {
		_logger->Error(message.c_str());
		if (message.find("Invalid market name provided") != string::npos) {
			auto s1 = message.find("'")+1;
			auto symbol = message.substr( s1, message.find("'", s1) - s1);
			_onEvent(
					getName(),
					symbol,
					marketlib::broker_event::subscribed_coin_is_not_valid, 
					(boost::format(R"(Subscription to %1% is not supported by %2%)") % symbol % getName()).str()
				);

			auto chnls = _channels
				.get<SubscribeChannelsByMarket>()
				.equal_range(boost::make_tuple(symbol));
			list<string> client_ids;
			for( auto chnl = chnls.first, end = chnls.second; chnl != end; ++chnl )
				client_ids.push_back(chnl->clientId);	
			for (const auto& a: client_ids)
				_channels.erase(
						_channels
						.get<SubscribeChannelsByClientAndMarket>()
						.find(boost::make_tuple(
							a,
							symbol )));
		}
		return;
	}
	std::string market = parsed_data.at("market").as_string().c_str();
	if (type  == "l3snapshot" || type  == "open") {
		auto addOrderToList = [&](const boost::json::value& set, std::list<ExecutionReport>& vec) {
            ExecutionReport order;
            order.clId=    set.at("clientId").as_string().c_str(); //strtoull(set.at("clientId").as_string().c_str(), nullptr, 0),
            order.exchId= set.at("orderId").as_string().c_str(); // atouint128(set.at("orderId").as_string().c_str())
            order.secId= "";
            order.cumQty= stod(set.at("size").as_string().c_str());
            order.leavesQty= stod(set.at("size").as_string().c_str());
            order.limitPrice= stod(set.at("price").as_string().c_str());
            order.side= stringToOrderSide(set.at("side").as_string().c_str());
            order.state= order_state_t::ost_New;
			order.type= report_type_t::rt_new;
            order.orderType= order_type_t::ot_Limit;
            vec.push_back(order);
		};

		if (type  == "l3snapshot") {
			_execution_reports[market] = std::list<ExecutionReport>{};
			auto& orders_list = _execution_reports[market];
			for (const auto &set : parsed_data.at("asks").as_array()) {
				addOrderToList(set, orders_list);
			};
			for (auto set : parsed_data.at("bids").as_array()) {
				addOrderToList(set, orders_list);
			};
		}
		else {
			addOrderToList(parsed_data, _execution_reports[market]);
			broadcastForMarketSubscribers(market, *(--_execution_reports[market].end()));
		}
		
	} else if (type  == "change") {
		auto& orders_lst = _execution_reports[market];
		auto exch_id = parsed_data.at("orderId").as_string().c_str();
		auto order = find_if(orders_lst.begin(), orders_lst.end(), [exch_id](auto a) {
			return a.exchId == exch_id;
		});
		order->side = stringToOrderSide(parsed_data.at("side").as_string().c_str());
		order->cumQty = stod(parsed_data.at("size").as_string().c_str());
		order->leavesQty = stod(parsed_data.at("size").as_string().c_str());
		order->limitPrice = stod(parsed_data.at("price").as_string().c_str());
		order->state= order_state_t::ost_Replaced;
		order->type= report_type_t::rt_replaced;
		broadcastForMarketSubscribers(market, *order);
	} 
	else if (type  == "fill") {
		auto& orders_lst = _execution_reports[market];
		auto exch_id = parsed_data.at("orderId").as_string().c_str();
		auto order = find_if(orders_lst.begin(), orders_lst.end(), [exch_id](auto a) {
			return a.exchId == exch_id;
		});

		if (order != orders_lst.end())
		{
			order->state = order_state_t::ost_Filled;
			order->type = report_type_t::rt_fill_trade;
			order->lastShares = stod(parsed_data.at("size").as_string().c_str());
			order->leavesQty = 0;
			order->lastPx = stod(parsed_data.at("price").as_string().c_str());
			return;
		}

		auto report = ExecutionReport();
		report.clId = parsed_data.at("clientId").as_string().c_str();
		report.exchId = exch_id;
		report.orderType = order_type_t::ot_Limit;
		report.type = report_type_t::rt_fill_trade;
		report.state = order_state_t::ost_Filled;
		report.limitPrice = stod(parsed_data.at("price").as_string().c_str());
		report.cumQty = stod(parsed_data.at("size").as_string().c_str());
		report.lastShares = stod(parsed_data.at("size").as_string().c_str());
		report.lastPx = stod(parsed_data.at("price").as_string().c_str());
		report.side = stringToOrderSide(parsed_data.at("side").as_string().c_str());
		orders_lst.push_back(report);
		// broadcastForMarketSubscribers(market, report);
	}
	else if (type == "done") {
		auto& orders_lst = _execution_reports[market];
		auto exch_id = parsed_data.at("orderId").as_string().c_str();
		auto order = find_if(orders_lst.begin(), orders_lst.end(), [exch_id](auto a) {
			return a.exchId == exch_id;
		});

		/*"done" can be pushed for orders that were never open in the order book 
		in the first place (ImmediateOrCancel orders for example)*/
		bool is_canceled = string(parsed_data.at("reason").as_string().c_str()) == string("canceled");
		if (order == orders_lst.end()) {
			auto report = ExecutionReport();
			report.clId = parsed_data.at("clientId").as_string().c_str();
			report.exchId = exch_id;
			report.orderType = order_type_t::ot_Limit;
			report.type = is_canceled ? report_type_t::rt_canceled : report_type_t::rt_fill_trade;
			report.state = is_canceled ? order_state_t::ost_Canceled : order_state_t::ost_Filled;
			report.limitPrice = 0;
			report.cumQty = 0;
			report.lastShares = 0;
			report.lastPx = 0;
			report.side = stringToOrderSide(parsed_data.at("side").as_string().c_str());

			broadcastForMarketSubscribers(market, report);
			return;
		}
		// logger->Info(message.c_str());
		// double remaining = is_canceled ? stod(parsed_data.at("sizeRemaining").as_string().c_str()) : 0;

		order->type = is_canceled ? report_type_t::rt_canceled : report_type_t::rt_fill_trade;
		order->state = is_canceled ? order_state_t::ost_Canceled : order_state_t::ost_Filled;
		if (is_canceled) 
			order->leavesQty = stod(parsed_data.at("sizeRemaining").as_string().c_str());
		else
			order->leavesQty = 0;
		broadcastForMarketSubscribers(market, *order);
		if (_execution_reports.find(market) != _execution_reports.end()) {
			orders_lst.erase(order);
		}
	}
}

void SerumTrade::broadcastForMarketSubscribers(const string& market, const ExecutionReport& report) const {
	auto chnls = _channels
			.get<SubscribeChannelsByMarket>()
			.equal_range(boost::make_tuple(market));

	auto next = chnls.first;
	auto current = chnls.first;
	while(next != chnls.second) {
		next++;
		current->callback(
			getName(),
			market,
			report
		);
		current = next;
	}
}

void SerumTrade::onUpdateHandler(const string &message) {
	
}

bool SerumTrade::enabledCheck() const {
	if (!isEnabled()) {
		_logger->Warn("Attempt to request disabled client");
	}
	return isEnabled();
}

bool SerumTrade::connectedCheck() const {
	if (!isConnected()) {
		_logger->Warn("Attempt to request disconnected client");
	}
	return isConnected();
}

bool SerumTrade::activeCheck() const {
	return enabledCheck() && connectedCheck();
}

SerumTrade::SerumTrade(logger_ptr logger, settings_ptr settings, callback_on_event OnEvent):
	_logger(logger), _connection(this, settings->get(ISettings::Property::WebsocketEndpoint), logger), 
	_settings(settings), _onEvent(OnEvent) {}

bool SerumTrade::isEnabled() const {
	return _connection.enabled;
}

bool SerumTrade::isConnected() const {
	return _connection.connected;
}

void SerumTrade::clearMarkets() {
#ifdef SERUM_LISTENER_DEBUG
	// logger_->Debug("> SerumTrade::clearMarkets");
#endif
	_execution_reports.clear();
	_channels.clear();
}

void SerumTrade::start() {
	_connection.async_start();
}
void SerumTrade::stop() {
	_connection.async_stop();
	clearMarkets();
}

void SerumTrade::listen(const SerumTrade::Instrument& instr, const string& clientId, callback_t callback) {
	auto chnls = _channels
		.get<SubscribeChannelsByMarket>()
		.equal_range(boost::make_tuple(
			getMarketFromInstrument(instr)
		));

	if (chnls.first == chnls.second) {
		_connection.async_send((boost::format(R"({
			"op": "subscribe",
			"channel": "level3",
			"markets": ["%1%"]
		})") % getMarketFromInstrument(instr)).str());
	}

	auto chnl_by_client = _channels
		.get<SubscribeChannelsByClientAndMarket>()
		.find(boost::make_tuple(
			clientId,
			getMarketFromInstrument(instr)
		));

	if (chnl_by_client == _channels.end()) {
		_channels.insert(
			SubscribeChannel{
				clientId,
				getMarketFromInstrument(instr),
				instr,
				callback
			}
		);
	}
	else {
		string msg = "The subscription with parameters already exists";
		msg += ": symbol - " + getMarketFromInstrument(instr);
		msg += ", client Id - " + clientId;

		_logger->Error( msg.c_str() );
	}
}

void SerumTrade::unlisten(const SerumTrade::Instrument& instr, const string& clientId) {
	auto chnl = _channels
		.get<SubscribeChannelsByClientAndMarket>()
		.find(boost::make_tuple(
			clientId,
			getMarketFromInstrument(instr)
		));

	if (chnl == _channels.end()) {
		_logger->Error("Subscription not found");
		return;
	}

	_channels.erase(chnl);
	auto chnls = _channels
		.get<SubscribeChannelsByMarket>()
		.equal_range(boost::make_tuple(
			getMarketFromInstrument(instr)
		));
	if (chnls.first == chnls.second) {
		_connection.async_send((boost::format(R"({
			"op": "unsubscribe",
			"channel": "level3",
			"markets": ["%1%"]
		})") % getMarketFromInstrument(instr)).str());

		_execution_reports.erase(getMarketFromInstrument(instr));
	}
}

void SerumTrade::unlistenForClientId(const string& clientId) {
	auto chnls = _channels
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
	return _settings->get(ISettings::Property::ExchangeName);
}

SerumTrade::~SerumTrade() {
	_connection.async_stop();
	while (isConnected()) continue;
}
