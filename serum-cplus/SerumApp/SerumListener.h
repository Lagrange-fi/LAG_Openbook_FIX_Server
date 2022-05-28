#pragma once

#include <unordered_map>
#include <memory>
#include <set>

#include "BrokerLib/IListener.h"
#include "BrokerLib/IBrokerApplication.h"
#include "Utility/ConnectionWrapper.h"
#include "BrokerLib/ISettings.h"
#include "BrokerLib/BrokerModels.h"

class SerumListener : public IListener
{
private:

	friend ConnectionWrapper < SerumListener >;

	typedef std::string string;
	typedef std::shared_ptr < ILogger > logger_ptr;
	typedef std::shared_ptr < ISettings > settings_ptr;
	typedef std::shared_ptr < IBrokerApplication > application_ptr;
	typedef BrokerModels::Market Market;
	typedef std::map < string,  std::list<BrokerModels::Order> > orders_map;

protected:

	struct SubscribeChannel {
		string id;
		string name;
		string pair;
		BrokerModels::Instrument instrument;
		std::shared_ptr < Market > market;
	};

	logger_ptr logger;
	application_ptr application;
	settings_ptr settings;
	ConnectionWrapper < SerumListener > connection;
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
	SerumListener(logger_ptr, application_ptr, settings_ptr);

	bool isEnabled() const override;
	bool isConnected() const override;
	// string getName() const override;

	void start() override;
	void stop() override;

	void listen(const BrokerModels::Instrument&) override;
	void unlisten(const BrokerModels::Instrument&) override;

	~SerumListener();

};
