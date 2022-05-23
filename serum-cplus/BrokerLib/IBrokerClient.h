#pragma once

#include <string>
#include "sharedlib/include/ILogger.h"
#include "BrokerModels.h"

class IBrokerClient {
private:
	typedef std::string string;
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
	enum class SubscriptionModel {
		TopBook,
		FullBook
	};

	IBrokerClient() {}
	IBrokerClient(const IBrokerClient&) = delete;
	IBrokerClient& operator = (const IBrokerClient&) = delete;

	virtual bool isEnabled() const = 0;
	virtual bool isConnected() const = 0;
	virtual string getName() const = 0;

	virtual void start() = 0;
	virtual void stop() = 0;

	virtual void subscribe(const BrokerModels::Instrument&, SubscriptionModel) = 0;
	virtual void unsubscribe(const BrokerModels::Instrument&, SubscriptionModel) = 0;

	virtual ~IBrokerClient() = default;

};
