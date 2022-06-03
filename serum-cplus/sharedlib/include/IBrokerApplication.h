#pragma once

#include <string>

#include "IBrokerClient.h"
#include <marketlib/include/BrokerModels.h>
#include <marketlib/include/market.h>

class IBrokerApplication {

private: 

	typedef std::string string;
	typedef IBrokerClient::BrokerEvent BrokerEvent;
	typedef marketlib::execution_report_t ExecutionReport;

public:

	virtual void onEvent(const string &exchangeName, BrokerEvent, const string &details) = 0;
	virtual void onReport(const string &exchangeName, const string &symbol, const BrokerModels::MarketBook&) = 0;
	virtual void onReport(const string &exchangeName, const string &symbol, const BrokerModels::DepthSnapshot&) = 0;
	virtual void onReport(const string& exchangeName, const string &symbol, const ExecutionReport&) = 0;

	virtual ~IBrokerApplication() = default;

};