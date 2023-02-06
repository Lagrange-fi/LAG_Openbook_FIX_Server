#pragma once

#include <string>
#include "ILogger.h"
#include <marketlib/include/BrokerModels.h>
#include <marketlib/include/enums.h>
#include <marketlib/include/market.h>
#include <vector>
#include <functional>
#include <any>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/composite_key.hpp>

class IBrokerClient {
private:
	typedef std::string string;
	typedef marketlib::market_depth_t SubscriptionModel;
	typedef marketlib::instrument_descr_t instrument;
	typedef std::function <void(const string&, const instrument&, const std::any&)> callback;
	// typedef std::function <void(const string&, const instrument&, const BrokerModels::MarketBook&)> callbackTop;
	// typedef std::function <void(const string&, const instrument&, const BrokerModels::DepthSnapshot&)> callbackDepth;
public:
	struct SubscribeChannel 
	{
		string clientId;
		string market;
		instrument instr;
		SubscriptionModel smodel;
		callback callback;
	};

	using SubscribedChannels = boost::multi_index::multi_index_container<
        SubscribeChannel,
        boost::multi_index::indexed_by<
            boost::multi_index::hashed_unique<
                boost::multi_index::tag<struct SubscribeChannelsByClientAndMarketAndSubscribeModel>,
                boost::multi_index::composite_key<
                    SubscribeChannel,
                    boost::multi_index::member<SubscribeChannel, decltype(SubscribeChannel::clientId), &SubscribeChannel::clientId>,
					boost::multi_index::member<SubscribeChannel, decltype(SubscribeChannel::market), &SubscribeChannel::market >,
					boost::multi_index::member<SubscribeChannel, decltype(SubscribeChannel::smodel), &SubscribeChannel::smodel >
                >
            >,
			boost::multi_index::hashed_non_unique<
                boost::multi_index::tag<struct SubscribeChannelsByMarketAndSubscribeModel>,
                boost::multi_index::composite_key<
                    SubscribeChannel,
					boost::multi_index::member<SubscribeChannel, decltype(SubscribeChannel::market), &SubscribeChannel::market >,
					boost::multi_index::member<SubscribeChannel, decltype(SubscribeChannel::smodel), &SubscribeChannel::smodel >
                >
            >,
			boost::multi_index::hashed_non_unique<
                boost::multi_index::tag<struct SubscribeChannelsByClient>,
                boost::multi_index::composite_key<
                    SubscribeChannel,
					boost::multi_index::member<SubscribeChannel, decltype(SubscribeChannel::clientId), &SubscribeChannel::clientId >
                >
            >
        >
    >;

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

	IBrokerClient() {}
	IBrokerClient(const IBrokerClient&) = delete;
	IBrokerClient& operator = (const IBrokerClient&) = delete;

	virtual bool isEnabled() const = 0;
	virtual bool isConnected() const = 0;
	virtual string getName() const = 0;

	virtual void start() = 0;
	virtual void stop() = 0;

	virtual void subscribe(const instrument&, SubscriptionModel,const string&, callback) = 0;
	// virtual void subscribe(const instrument&, const string&, callbackTop) = 0;
	// virtual void subscribe(const instrument&, const string&, callbackDepth) = 0;
	virtual void unsubscribe(const instrument&, SubscriptionModel, const string&) = 0;
	virtual void unsubscribeForClientId(const string&) = 0;
	virtual std::list< instrument > getInstruments() = 0;

	virtual ~IBrokerClient() = default;
};
