
#pragma once

#include <memory>
#include <fix8/f8includes.hpp>
#include <fix8/usage.hpp>

#include "FIX8_SERUM_Order_router.hpp"
#include "FIX8_SERUM_Order_types.hpp"
#include "FIX8_SERUM_Order_classes.hpp"

#include <marketlib/include/market.h>
#include <marketlib/include/BrokerModels.h>
#include <sharedlib/include/IBrokerClient.h>

class ILogger;
class ISettings;
class IBrokerClient;

class SERUM_Order_sandbox_session : public FIX8::Session ,
                            public FIX8::SERUM_Order::FIX8_SERUM_Order_Router{
public :
    SERUM_Order_sandbox_session(const FIX8::F8MetaCntx& ctx,
                       const FIX8::sender_comp_id& sci,
                       FIX8::Persister *persist=nullptr,
                       FIX8::Logger *logger=nullptr,
                       FIX8::Logger *plogger=nullptr);
    virtual ~SERUM_Order_sandbox_session();

private:

    // FIX8::Session implementation
    bool handle_application(const unsigned seqnum, const FIX8::Message *&msg) override;
    bool handle_logon(const unsigned seqnum, const FIX8::Message *msg) override;
    bool handle_logout(const unsigned seqnum, const FIX8::Message *msg) override;
    void modify_outbound(FIX8::Message *msg) override;
    bool process(const FIX8::f8String& from) override;
    FIX8::Message *generate_logon(const unsigned heartbeat_interval, const FIX8::f8String davi=FIX8::f8String()) override;

    // FIX8::SERUM_Data::FIX8_SERUM_Data_Router implementation
    virtual bool operator() (const class Message *msg) const { return false; } ;
    virtual bool operator() (const class Heartbeat *msg) const { return true; }
    virtual bool operator() (const class TestRequest *msg) const { return true; }
    virtual bool operator() (const class ResendRequest *msg) const { return true; }
    virtual bool operator() (const class Reject *msg) const { return true; }
    virtual bool operator() (const class SequenceReset *msg) const { return true; }
    virtual bool operator() (const class Logout *msg) const { return true; }
    virtual bool operator() (const class Logon *msg) const { return true; }
    bool operator() (const class FIX8::SERUM_Order::NewOrderSingle *msg) const override;
    bool operator() (const class FIX8::SERUM_Order::OrderCancelRequest *msg) const override;

    // IBrokerApplication implementation
    //void onEvent (const std::string &exchangeName, IBrokerClient::BrokerEvent, const std::string &details) override;
    //void onReport(const std::string& exchangeName, const std::string &symbol, const marketlib::execution_report_t&) override;

    // FIX response implementation
    void sendExecutionReport(const std::string &tradeId, const std::string &clId,
                             marketlib::report_type_t type, marketlib::order_state_t state, const std::string &exchId,
                             double lastPx, double lastShares);
    void sendReport(const std::string&clId,marketlib::report_type_t,marketlib::order_state_t,
                    const std::string&exId="",const std::string&origClId="", const std::string&text="") ;
    void sendCancelRejectReport(const std::string &clId, const std::string &text);

    const std::string& sess_id();

private:
    std::shared_ptr < ILogger > _logger;
    mutable  std::unordered_map<std::string, marketlib::order_t>  _orders;
};
