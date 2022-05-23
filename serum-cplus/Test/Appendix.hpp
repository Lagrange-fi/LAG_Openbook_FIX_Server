#pragma once

#include "BrokerLib/IBrokerApplication.h"
#include "BrokerLib/ISettings.h"
#include "sharedlib/include/ILogger.h"
#include <boost/format.hpp>

class Logger: public ILogger
{
private:

	typedef std::string string;

public:

	void Info(const char *content, ...) override;
	void Debug(const char *content, ...) override;
	void Error(const char *content, ...) override;
	void Warn(const char *content, ...) override;
    void Critical(const char *content, ...) override;
    void Trace(const char *content, ...) override;

	~Logger() = default;
};

void Logger::Info(const char *content, ...) {
	cout << "INFO | " << content << "\n";
}
void Logger::Debug(const char *content, ...) {
	cout << "INFO | " << content << "\n";
}
void Logger::Error(const char *content, ...) {
	cout << "INFO | " << content << "\n";
}
void Logger::Warn(const char *content, ...) {
	cout << "INFO | " << content << "\n";
}

void Logger::Critical(const char *content, ...) {
	cout << "CRITICAL | " << content << "\n";
}

void Logger::Trace(const char *content, ...) {
	cout << "TRACE | " << content << "\n";
}



class SerumSettings : public ISettings {

private:

    typedef std::string string;

public:

    string get(Property property) const override {
        switch (property) {
        case Property::ExchangeName:
            return "Serum";
        case Property::WebsocketEndpoint:
            return "wss://api.serum-vial.dev/v1/ws";
        default:
            return "";
        }
    }
};



class BrokerNullApplication : public IBrokerApplication	 {

private:

	typedef std::shared_ptr < ILogger > logger_ptr;
	typedef std::string string;
	typedef BrokerModels::Market Market;
    typedef IBrokerClient::BrokerEvent BrokerEvent;

protected:

	logger_ptr logger;

public:

	BrokerNullApplication(logger_ptr);
    void onEvent(const string &exchangeName, BrokerEvent, const string &details) override;
	void onReport(const string &exchangeName, const string &symbol, const BrokerModels::MarketBook&) override;
    void onReport(const string &exchangeName, const string &symbol, const BrokerModels::DepthSnapshot&) override;
	~BrokerNullApplication() = default;

};


BrokerNullApplication::BrokerNullApplication(logger_ptr _logger): logger(_logger) {}

void BrokerNullApplication::onEvent(const string &exchangeName, BrokerEvent, const string &details) {
    logger->Debug((boost::format("> BrokerNullApplication::onEvent => Exchange '%1%'") % exchangeName).str().c_str());
	logger->Info(details.c_str());
}

void BrokerNullApplication::onReport(const string &exchangeName, const string &symbol, const MarketBook &marketBook) {    
    logger->Info((boost::format("%1%\nAsk(%2%) AskSize(%3%) --- Bid(%4%) BidSize(%5%)") 
        % symbol 
        % marketBook.askPrice 
        % marketBook.askSize 
        % marketBook.bidPrice 
        % marketBook.bidSize).str().c_str());
}

void BrokerNullApplication::onReport(const string &exchangeName, const string &symbol, const BrokerModels::DepthSnapshot& depth){
    int count = 7;
    std::ostringstream strs;
    strs << symbol << "\nAsks\n";
    for (auto ask = depth.asks.begin() + count - 1; ask >= depth.asks.begin(); ask--) {
        strs << (*ask).volume << "  " << (*ask).price << std::endl;
    }
    strs << "\nBids\n";
    for (auto bid = depth.bids.begin(); bid < depth.bids.begin() + count; bid++) {
        strs << (*bid).volume << "  " << (*bid).price << std::endl;
    }

    logger->Info(strs.str().c_str());
};

