#pragma once

#include <string>

#include "IBrokerClient.h"
#include "BrokerModels.h"

class IBrokerApplication {

private: 

	typedef std::string string;
	typedef IBrokerClient::BrokerEvent BrokerEvent;

public:

	virtual void onEvent(const string &exchangeName, BrokerEvent, const string &details) = 0;
	virtual void onReport(const string &exchangeName, const string &symbol, const BrokerModels::MarketBook&) = 0;
	virtual void onReport(const string &exchangeName, const string &symbol, const BrokerModels::DepthSnapshot&) = 0;

	virtual ~IBrokerApplication() = default;

};