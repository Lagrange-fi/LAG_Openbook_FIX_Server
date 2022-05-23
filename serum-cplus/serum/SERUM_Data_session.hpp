#pragma once

#include <fix8/f8includes.hpp>
#include <fix8/usage.hpp>

#include "FIX8_SERUM_Data_router.hpp"
#include "FIX8_SERUM_Data_types.hpp"
#include "FIX8_SERUM_Data_classes.hpp"

#include <marketlib/include/market.h>
#include <sharedlib/include/Logger.h>

class SERUM_Data_session : public FIX8::Session , public FIX8::SERUM_Data::FIX8_SERUM_Data_Router {
public:
    SERUM_Data_session(const FIX8::F8MetaCntx& ctx,
                       const FIX8::sender_comp_id& sci,
                      FIX8::Persister *persist=nullptr,
                      FIX8::Logger *logger=nullptr,
                      FIX8::Logger *plogger=nullptr);

    void setConfig(const XmlElement * session_cfg)noexcept{_session_cfg=session_cfg;}
    void setLogger(const LoggerPtr& logger){_logger=logger;}

    bool handle_application(const unsigned seqnum, const FIX8::Message *&msg) override;
    bool handle_logon(const unsigned seqnum, const FIX8::Message *msg) override;
    bool handle_logout(const unsigned seqnum, const FIX8::Message *msg) override;
    void modify_outbound(FIX8::Message *msg) override;
    bool process(const FIX8::f8String& from) override;
    FIX8::Message *generate_logon(const unsigned heartbeat_interval, const FIX8::f8String davi=FIX8::f8String()) override;

private:
    virtual bool operator() (const class Message *msg) const { return false; }
    virtual bool operator() (const class Heartbeat *msg) const { return true; }
    virtual bool operator() (const class TestRequest *msg) const { return true; }
    virtual bool operator() (const class ResendRequest *msg) const { return true; }
    virtual bool operator() (const class Reject *msg) const { return true; }
    virtual bool operator() (const class SequenceReset *msg) const { return true; }
    virtual bool operator() (const class Logout *msg) const { return true; }
    virtual bool operator() (const class Logon *msg) const { return true; }
    bool operator() (const class FIX8::SERUM_Data::SecurityListRequest *msg) const override;
    bool operator() (const class FIX8::SERUM_Data::MarketDataRequest *msg) const override;

private:
    const XmlElement * _session_cfg;
    bool _display_debug=true;
    LoggerPtr _logger;
};
