#pragma once

#include <string>
#include "ILogger.h"
#include <marketlib/include/BrokerModels.h>
#include <marketlib/include/enums.h>
#include <marketlib/include/market.h>
#include <vector>

class IBrokerClient {
private:
	typedef std::string string;
	typedef marketlib::market_depth_t SubscriptionModel;
	typedef marketlib::instrument_descr_t instrument;
public:

	enum class BrokerEvent {
		Info,
		Debug,
		Error,
		SessionLogon,
		SessionLogout,
		CoinSubscribed,
		CoinUnsubscribed,
		ConnectorStarted,
		ConnectorStopped,
		CoinSubscribedFault,
		CoinUnsubscribedFault,
		SubscribedCoinIsNotValid
	};
	// enum class SubscriptionModel {
	// 	TopBook,
	// 	FullBook
	// };

	IBrokerClient() {}
	IBrokerClient(const IBrokerClient&) = delete;
	IBrokerClient& operator = (const IBrokerClient&) = delete;

	virtual bool isEnabled() const = 0;
	virtual bool isConnected() const = 0;
	virtual string getName() const = 0;

	virtual void start() = 0;
	virtual void stop() = 0;

	virtual void subscribe(const instrument&, SubscriptionModel) = 0;
	virtual void unsubscribe(const instrument&, SubscriptionModel) = 0;
	virtual std::vector< instrument > getInstruments() = 0;

	virtual ~IBrokerClient() = default;

};
