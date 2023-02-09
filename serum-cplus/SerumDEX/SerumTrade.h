#pragma once

#include <unordered_map>
#include <memory>
#include <set>

#include "SerumAdapter.h"
#include <fstream>

#include <sharedlib/include/ConnectionWrapper.h>
#include <sharedlib/include/ISettings.h>
#include <sharedlib/include/IListener.h>
#include <marketlib/include/BrokerModels.h>
#include <marketlib/include/market.h>
#include <marketlib/include/enums.h>

#include <instruments/converter.hpp>


class SerumTrade : public IListener
{
private:

	friend ConnectionWrapper < SerumTrade >;

	typedef std::string string;
	typedef std::shared_ptr < ILogger > logger_ptr;
	typedef std::shared_ptr < ISettings > settings_ptr;
	typedef BrokerModels::Market Market;
	typedef marketlib::order_t Order;
	typedef marketlib::execution_report_t ExecutionReport;
	typedef std::map < string, std::list< Order > > orders_map;
	typedef marketlib::instrument_descr_t Instrument;
	typedef std::function <void(const string&, const string&, const ExecutionReport&)> callback_t;
	typedef std::function <void(const string &exchangeName, marketlib::broker_event, const string &details)> callback_on_event;

protected:
	logger_ptr _logger;
	settings_ptr _settings;
	ConnectionWrapper < SerumTrade > _connection;
	orders_map _orders;
	SubscribedChannels _channels;
	callback_on_event _onEvent;
	

	void onOpen();
	void onClose();
	void onFail();
	void onMessage(const string&);

	void onEventHandler(const string&);
	void onUpdateHandler(const string&);

	void clearMarkets();

	bool enabledCheck() const;
	bool connectedCheck() const;
	bool activeCheck() const;

public:
	SerumTrade(logger_ptr, settings_ptr, callback_on_event);

	bool isEnabled() const override;
	bool isConnected() const override;
	string getName() const override;

	void start() override;
	void stop() override;

	void listen(const Instrument&, const string&, callback_t) override;
	void unlisten(const Instrument&, const string&) override;
	void unlistenForClientId(const string&) override;

	~SerumTrade();

};
