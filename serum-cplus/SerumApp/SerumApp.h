#pragma once

#include <unordered_map>
#include <memory>
#include <set>

#include "BrokerLib/IBrokerClient.h"
#include "BrokerLib/IBrokerApplication.h"
#include "Utility/ConnectionWrapper.h"
#include "BrokerLib/ISettings.h"
#include "BrokerLib/BrokerModels.h"

class SerumApp : public IBrokerClient {

private:

	friend ConnectionWrapper < SerumApp >;

	typedef std::string string;
	typedef std::shared_ptr < ILogger > logger_ptr;
	typedef std::shared_ptr < ISettings > settings_ptr;
	typedef std::shared_ptr < IBrokerApplication > application_ptr;
	typedef BrokerModels::Market Market;
	typedef std::map < string,  BrokerModels::DepthSnapshot > depth_snapshots;

protected:

	struct SubscribeChannel {
		string id;
		string name;
		string pair;
		BrokerModels::Instrument instrument;
		std::shared_ptr < Market > market;
	};

	logger_ptr logger;
	settings_ptr settings;
	application_ptr application;
	ConnectionWrapper < SerumApp > connection;
	depth_snapshots depth_snapshot;
	string name;
	

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
	SerumApp(logger_ptr, application_ptr, settings_ptr);

	bool isEnabled() const override;
	bool isConnected() const override;
	string getName() const override;

	void start() override;
	void stop() override;

	void subscribe(const BrokerModels::Instrument&, SubscriptionModel) override;
	void unsubscribe(const BrokerModels::Instrument&, SubscriptionModel) override;

	~SerumApp();

};

