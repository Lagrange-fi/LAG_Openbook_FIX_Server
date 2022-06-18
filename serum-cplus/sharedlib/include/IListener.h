#pragma once

#include <string>
#include "ILogger.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <marketlib/include/BrokerModels.h>
#include <marketlib/include/market.h>

class IListener {
private:
	typedef std::string string;
	typedef marketlib::instrument_descr_t Instrument;
	typedef marketlib::execution_report_t ExecutionReport;
	typedef std::function <void(const string&, const string&, const ExecutionReport&)> callback_t;

public:

	struct SubscribeChannel 
	{
		string clientId;
		string market;
		callback_t callback;
	};

	using SubscribedChannels = boost::multi_index::multi_index_container<
        SubscribeChannel,
        boost::multi_index::indexed_by<
            boost::multi_index::hashed_unique<
                boost::multi_index::tag<struct SubscribeChannelsByClientAndMarket>,
                boost::multi_index::composite_key<
                    SubscribeChannel,
                    boost::multi_index::member<SubscribeChannel, decltype(SubscribeChannel::clientId), &SubscribeChannel::clientId>,
					boost::multi_index::member<SubscribeChannel, decltype(SubscribeChannel::market), &SubscribeChannel::market >
                >
            >,
			boost::multi_index::hashed_non_unique<
                boost::multi_index::tag<struct SubscribeChannelsByMarket>,
                boost::multi_index::composite_key<
                    SubscribeChannel,
					boost::multi_index::member<SubscribeChannel, decltype(SubscribeChannel::market), &SubscribeChannel::market >
                >
            >
        >
    >;

	IListener() {}
	IListener(const IListener&) = delete;
	IListener& operator = (const IListener&) = delete;

	virtual bool isEnabled() const = 0;
	virtual bool isConnected() const = 0;
	// virtual string getName() const = 0;

	virtual void start() = 0;
	virtual void stop() = 0;

	virtual void listen(const Instrument&, const string&, callback_t) = 0;
	virtual void unlisten(const Instrument&, const string&) = 0;

	virtual ~IListener() = default;
};
