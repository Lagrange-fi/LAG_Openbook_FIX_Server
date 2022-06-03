#pragma once

#include <unordered_map>
#include <memory>
#include <set>

#include <sharedlib/include/IBrokerApplication.h>
#include <sharedlib/include/ConnectionWrapper.h>
#include <sharedlib/include/ISettings.h>
#include <sharedlib/include/IListener.h>
#include <marketlib/include/BrokerModels.h>
#include <marketlib/include/market.h>


class SerumTrade : public IListener
{
private:

	friend ConnectionWrapper < SerumTrade >;

	typedef std::string string;
	typedef std::shared_ptr < ILogger > logger_ptr;
	typedef std::shared_ptr < ISettings > settings_ptr;
	typedef std::shared_ptr < IBrokerApplication > application_ptr;
	typedef BrokerModels::Market Market;
	typedef marketlib::order_t Order;
	typedef marketlib::execution_report_t ExecutionReport;
	typedef std::map < string, std::list< Order > > orders_map;
	typedef marketlib::instrument_descr_t Instrument;


protected:

	// struct SubscribeChannel {
	// 	string id;
	// 	string name;
	// 	string pair;
	// 	BrokerModels::Instrument instrument;
	// 	std::shared_ptr < Market > market;
	// };

	logger_ptr logger;
	application_ptr application;
	settings_ptr settings;
	ConnectionWrapper < SerumTrade > connection;
	orders_map orders;
	

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
	SerumTrade(logger_ptr, application_ptr, settings_ptr);

	bool isEnabled() const override;
	bool isConnected() const override;
	// string getName() const override;

	void start() override;
	void stop() override;

	void listen(const Instrument&) override;
	void unlisten(const Instrument&) override;

	~SerumTrade();

};
