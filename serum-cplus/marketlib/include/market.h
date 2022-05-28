#pragma once

#include <string>
#include <memory>
#include <chrono>

#include "enums.h"

namespace marketlib
{
    struct market_data_request_t
    {
        const std::string   engine;
        const std::string   symbol;
        const market_depth_t depth;/* top,full*/
        const subscription_update_type_t update_type;/*full, incremental*/
    };

    /*
    * @class instrument_descr_
    * @brief instrument_descr_t represent fields of pair
    */
    struct instrument_descr_t
    {
        const std::string   engine;
        std::string     sec_id;
        std::string    symbol;
        std::string     currency;
        int            tick_precision;
        const std::string& hash () const		{ return  sec_id;}
    };

    using InstrumentPtr = std::shared_ptr<instrument_descr_t>;
    /*
    * @class md_snapshot_t
    * @brief md_snapshot_t represent top of book update
    */
    struct md_snapshot_t
    {
        std::chrono::time_point<std::chrono::system_clock> time;
        double	 bid;
        double   ask;
        double   trade;
        unsigned bid_size;
        unsigned ask_size;
        unsigned trade_size;
        int      pad[7]={};
        md_snapshot_t ():
                time(),
                bid(-1.0),
                ask(-1.0),
                trade(-1.0),
                bid_size(0),
                ask_size(0),
                trade_size(0){
        }
        bool is_bid() const{return bid != -1.0 && bid != 0.0;}
        bool is_ask() const{return ask != -1.0 && ask != 0.0;}
    };


    /*
  * @class md_snapshot_t
  * @brief md_snapshot_t represent execution report from a broker
  */
    struct execution_report_t
    {
        std::string     tradeId ;
        std::string     clId ;
        std::string     origClId;
        std::string     exchId;
        std::string     secId;
        time_t          time ;
        std::string     text;
        order_type_t    orderType ;
        report_type_t   type;
        exec_trans_t    transType;
        time_in_force_t tif;
        order_state_t   state;
        order_side_t    side ;
        ord_rej_reason  rejReason;
        double          commision ;
        double          stopPrice;
        double          limitPrice;
        double          avgPx;
        double          lastPx;
        double          leavesQty;
        double          cumQty;
        double          lastShares ;
        double          ping_milli;
        execution_report_t():
                tradeId(),
                clId(),
                origClId(),
                secId(),
                time(0),
                text(),
                orderType(ot_Undefined),
                type(rt_undefined),
                transType(ett_undefined),
                tif(tf_Undefined),
                state(ost_Undefined),
                side(os_Undefined),
                commision(0.0),
                stopPrice(0.0),
                limitPrice(0.0),
                avgPx(0.0),
                lastPx(0.0),
                leavesQty(0.0),
                cumQty(0.0),
                lastShares(0.0),
                ping_milli(0.0){
        }
    };

    typedef size_t rule_id;
    struct order_t
    {
        std::string clId;
        std::string exchId;
        std::string secId;
        double original_qty  = 0.0;
        double remaining_qty = 0.0;
        double price     = 0.0;
        double stopPrice = 0.0;
        order_side_t    side  = os_Undefined;
        order_state_t   state = ost_Undefined;
        time_in_force_t tif   = tf_Undefined;
        order_type_t type  = ot_Undefined;
        time_t init_time = 0;
        time_t last_time = 0;
        rule_id  ruleId=0;  //for stop orders only
        unsigned ruleLevel=0;//for stop orders only
        bool isCompleted() const noexcept {
            return state==order_state_t::ost_Filled
            ||state==order_state_t::ost_Canceled
            ||state==order_state_t::ost_Cancel_Rejected;}
    };

    struct order_update_t
    {
        std::string clId;
        std::string origId;
        std::string exchId;
        double price ;
        double stopPrice ;
        double qty ;
    };

    struct position_t
    {
        std::string broker;
        std::string sec_id;
        double position;
        double average;
    };

    struct ohlc_t
    {
        ohlc_t()=delete;
        ohlc_t(double last, time_t t):
            open(last),high(last),low(last),close(last),time(t){
        }
        double open ;
        double high ;
        double low  ;
        double close;
        time_t time ;
    };

}

