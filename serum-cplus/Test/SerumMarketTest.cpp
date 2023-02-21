#include <iostream>
#include <string>
#include <atomic>

#include <SerumDEX/SerumMarket/Market.hpp>
#include <SerumDEX/SerumMarket/models.hpp>
#include <SerumDEX/SerumTrade.h>
#include <SerumDEX/PoolRequester/PoolsRequester.h>
#include <marketlib/include/BrokerModels.h>
#include <marketlib/include/enums.h>
#include <sharedlib/include/IListener.h>
#include "Appendix.hpp"
#include "settings.h"

using namespace std;
using namespace BrokerModels;
using namespace marketlib;
typedef marketlib::order_t Order;
typedef instrument_descr_t Instrument;


std::string str_state(marketlib::order_state_t state){
    switch (state) {
        case '0': return "NewOrder";
        case '1': return "PartiallyFilled";
        case '2': return "Filled";
        case '4': return "Cancelled";
        case '8': return "Rejected";
        default: return "Undefined";
    }
}


int main () 
{
    shared_ptr < ILogger > logger(new Logger);
    shared_ptr < ISettings > settings(new SerumSettings);
    shared_ptr < IPoolsRequester > pools(new PoolsRequester(logger, settings, "./market.json"));
    shared_ptr < IListener >  trade_channel (new SerumTrade ( logger, settings, [&logger](const string& exch, broker_event event, const string& info) {logger->Trace(info.c_str());}));
    trade_channel->start();
    auto instr = Instrument{
        engine: "",
        sec_id: "",
        symbol: "SOL/USDC",
        base_currency: "SOL",
        quote_currency: "USDC"
    };

    auto market = SerumMarket(
        PUBKEY, 
        SECRETKEY, 
        "https://nd-664-169-151.p2pify.com/a89ccd991de179587a0b8e3356409a9b", 
        pools,
        trade_channel,
        [](const string& name, const Instrument& inst, const string& info){
            std::cout << name << " || " << inst.symbol << " || " + info;},
        [](const string& name, const execution_report_t& execution_report) {
            std::cout << "Order Update" << std::endl;
            std::cout << "id:" << execution_report.clId << std::endl;
            std::cout << "status" << str_state(execution_report.state) << endl;  
        },
        "Market_1"
    );

    // cout << sizeof(Instruction) << endl;
    Instrument instrument{"", "", "SOL/USDC", "SOL", "USDC" };
    order_t order_sell;
    order_sell.price = 39;
    order_sell.original_qty = 0.1;
    order_sell.side = order_side_t::os_Sell;
    order_sell.clId = "1342";
    // order_sell.exchId = "719423018874672537328158";

    order_t order_buy;
    order_buy.price = 15;
    order_buy.original_qty = 0.1;
    order_buy.side = order_side_t::os_Buy;
    order_buy.clId = "7654321";

    while (1)
    {
        string msg;
        cin >> msg;
        if (msg == "so") {
            order_sell = market.send_new_order(instrument, order_sell);
        }
        else if (msg == "sc") {
            order_sell = market.cancel_order(instrument, order_sell);
        }
        else if (msg == "bo") {
           order_buy = market.send_new_order(instrument, order_buy);
        }
        else if (msg == "bc") {
            order_buy = market.cancel_order(instrument, order_buy);
        }
        else if (msg == "stop") {
            break;
        }
    }
    
    
    // market.cancel_order(instrument, order);
}