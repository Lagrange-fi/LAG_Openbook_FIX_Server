#pragma once

#include <sharedlib/include/IBrokerApplication.h>
#include <sharedlib/include/ISettings.h>
#include <sharedlib/include/ILogger.h>
#include <marketlib/include/market.h>
#include <boost/format.hpp>

using namespace std;
using namespace marketlib;
using namespace BrokerModels;


class Logger: public ILogger
{
private:

	typedef std::string string;

public:

	void Info(const char *content, ...) override;
	void Debug(const char *content, ...) override;
	void Error(const char *content, ...) override;
    void Critical(const char *content, ...) override;
	void Warn(const char *content, ...) override;
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
    typedef marketlib::execution_report_t ExecutionReport;

protected:

	logger_ptr logger;

public:

	BrokerNullApplication(logger_ptr);
    void onEvent(const string &exchangeName, BrokerEvent, const string &details) override;
	void onReport(const string &exchangeName, const string &symbol, const BrokerModels::MarketBook&) override;
    void onReport(const string &exchangeName, const string &symbol, const BrokerModels::DepthSnapshot&) override;
    void onReport(const string& exchangeName, const string &symbol, const ExecutionReport&) override;
	~BrokerNullApplication() = default;

};


BrokerNullApplication::BrokerNullApplication(logger_ptr _logger): logger(_logger) {}

void BrokerNullApplication::onEvent(const string &exchangeName, BrokerEvent, const string &details) {
    logger->Debug((boost::format("> BrokerNullApplication::onEvent => Exchange '%1%'") % exchangeName).str().c_str());
	logger->Info(details.c_str());
}

void BrokerNullApplication::onReport(const string &exchangeName, const string &symbol, const MarketBook &marketBook) {
    time_t update_time = std::chrono::system_clock::to_time_t(marketBook.time);
    struct tm *_tm = gmtime(&update_time);
    std::chrono::system_clock::duration duration = marketBook.time.time_since_epoch();
    auto milli_total = std::chrono::duration_cast<std::chrono::milliseconds> (duration).count();
    int milli_since = milli_total % 1000;
    char buff[64];
    //  "timestamp": "2021-03-24 01:00:55.586",
    sprintf(buff, "%.4d%.2d%.2d %.2d:%.2d:%.2d.%.3d",
            _tm->tm_year+1900, _tm->tm_mon, _tm->tm_mday,_tm->tm_hour,_tm->tm_min,_tm->tm_sec, milli_since);

    logger->Info((boost::format("%1%\nAsk(%2%) AskSize(%3%) --- Bid(%4%) BidSize(%5%), update_time(%6%)")
        % symbol 
        % marketBook.askPrice 
        % marketBook.askSize 
        % marketBook.bidPrice 
        % marketBook.bidSize
        % buff).str().c_str());
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

void BrokerNullApplication::onReport(const string& exchangeName, const string &symbol, const ExecutionReport& report) {
    std::string state = "";
    if ( report.state == order_state_t::ost_New ) {
        state = "new";
    }
    else if ( report.state == order_state_t::ost_Filled ) {
        state = "filled";
    }
    else if ( report.state == order_state_t::ost_Canceled ) {
        state = "cancelled";
    }
    else if ( report.state == order_state_t::ost_Replaced ) {
        state = "replaced";
    }


    logger->Info((boost::format("Symbol(%1%)\nType(%2%)\nSide(%3%)\nPrice(%4%)\nAmountExecuted(%5%)\nAmountRemaining(%6%)\nStatus(%7%)\nExchId(%8%)\n\n") 
        % symbol
        % (report.orderType == order_type_t::ot_Limit ? "limit" : "market")
        % (report.side == order_side_t::os_Buy ? "buy" : "sell")
        % report.limitPrice 
        % report.cumQty
        % report.leavesQty
        % state
        % report.exchId
    ).str().c_str());
}