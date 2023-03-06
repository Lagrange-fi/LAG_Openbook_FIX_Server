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
    shared_ptr < IPoolsRequester > pools(new PoolsRequester(logger, settings));
    shared_ptr < IListener >  trade_channel (new SerumTrade ( logger, settings, [&logger](const string& exch, const string& synbol, broker_event event, const any& info) {logger->Trace(any_cast<string>(info).c_str());}));
    trade_channel->start();
    // auto instr = Instrument{
    //     engine: "",
    //     sec_id: "",
    //     symbol: "SOL/USDC",
    //     base_currency: "SOL",
    //     quote_currency: "USDC"
    // };

    auto market = SerumMarket(
        PUBKEY, 
        SECRETKEY, 
        "https://nd-664-169-151.p2pify.com/a89ccd991de179587a0b8e3356409a9b",
        logger,
        pools,
        trade_channel,
        [](const string& name, const execution_report_t& execution_report) {
            // std::cout << "Order Update" << std::endl;
            // std::cout << "id:" << execution_report.clId << std::endl;
            // std::cout << "status" << str_state(execution_report.state) << endl; 
            std::cout << execution_report.tradeId << endl
            << execution_report.clId << endl
            << execution_report.origClId << endl
            << execution_report.exchId << endl
            << execution_report.secId << endl
            << execution_report.transaction_hash << endl
            << str_state(execution_report.state) << endl
            << execution_report.text << endl;
        },
        "Market_1"
    );

    // cout << sizeof(Instruction) << endl;
    Instrument instrument{"", "", "R/USDC" };
    order_t order_sell;
    order_sell.price = 0.35;
    order_sell.original_qty = 0.1;
    order_sell.side = order_side_t::os_Sell;
    order_sell.clId = "1342";
    // order_sell.type = order_type_t::ot_Limit;
    // order_sell.exchId = "719423018874672537328158";

    order_t order_buy;
    order_buy.price = 0.29;
    order_buy.original_qty = 0.1;
    order_buy.side = order_side_t::os_Buy;
    order_buy.clId = "7654321";
    order_buy.type = order_type_t::ot_Limit;

    while (1)
    {
        string msg;
        cin >> msg;
        if (msg == "so") {
            market.send_new_order(instrument, order_sell);
        }
        else if (msg == "sc") {
            market.cancel_order(instrument, order_sell.clId);
        }
        else if (msg == "bo") {
            order_buy.clId = "7654321";
            market.send_new_order(instrument, order_buy);
        }
        else if (msg == "bc") {
            order_buy.clId = "7654321";
            market.cancel_order(instrument, order_buy.clId);
        }
        else if (msg == "bo1") {
            order_buy.clId = "1234567";
            market.send_new_order(instrument, order_buy);

        }
        else if (msg == "bc1") {
            order_buy.clId = "1234567";
            market.cancel_order(instrument, order_buy.clId);
        }
        else if (msg == "stop") {
            break;
        }
    }
    
    
    // market.cancel_order(instrument, order);
}