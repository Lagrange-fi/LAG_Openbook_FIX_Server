#pragma once
#include "SerumAdapter.h"

#include <unordered_map>
#include <memory>
#include <set>
#include <vector>
#include <list>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <any>


#include <sharedlib/include/IBrokerClient.h>
#include <sharedlib/include/ConnectionWrapper.h>
#include <sharedlib/include/ISettings.h>
#include <sharedlib/include/IPoolsRequester.h>
#include <marketlib/include/BrokerModels.h>
#include <marketlib/include/enums.h>

class SerumMD : public IBrokerClient {

private:

	friend ConnectionWrapper < SerumMD >;

	typedef std::string string;
	typedef std::shared_ptr < ILogger > logger_ptr;
	typedef std::shared_ptr < ISettings > settings_ptr;
	typedef std::shared_ptr < IPoolsRequester > pools_ptr;
	typedef BrokerModels::Market Market;
	typedef std::map < string,  BrokerModels::DepthSnapshot > depth_snapshots;
	typedef std::map < string,  BrokerModels::MarketBook > top_snapshots;
	typedef marketlib::market_depth_t SubscriptionModel;
	typedef marketlib::instrument_descr_t instrument;
	typedef std::function <void(const string&, const instrument&, const std::any&)> callback;
	typedef std::function <void(const string &exchangeName, marketlib::broker_event, const string &details)> callback_on_event;

protected:

	// struct SubscribeChannel {
	// 	string id;
	// 	string name;
	// 	string pair;
	// 	BrokerModels::Instrument instrument;
	// 	std::shared_ptr < Market > market;
	// };

	logger_ptr logger;
	settings_ptr settings;
	pools_ptr pools;
	SubscribedChannels channels;
	callback_on_event onEvent;
	ConnectionWrapper < SerumMD > connection;
	depth_snapshots depth_snapshot;
	top_snapshots top_snapshot;
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
	SerumMD(logger_ptr, settings_ptr, pools_ptr, callback_on_event);

	bool isEnabled() const override;
	bool isConnected() const override;
	string getName() const override;

	void start() override;
	void stop() override;

	// void subscribe(const instrument&, const string&, callbackTop) override;
	// void subscribe(const instrument&, const string&, callbackDepth) override;
	void subscribe(const instrument&, SubscriptionModel, const string&, callback) override;
	void unsubscribe(const instrument&, SubscriptionModel, const string&) override;
	void unsubscribeForClientId(const string&) override;
	std::list< instrument > getInstruments() override;

	~SerumMD();

private:
	void subscribe(const instrument&, SubscriptionModel);
};

