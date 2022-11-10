#include <iostream>
#include <string>
#include <atomic>

#include <SerumDEX/SerumMarket/market.h>
#include <SerumDEX/SerumMarket/structs.h>
#include <SerumDEX/SerumPoolsRequester.h>
#include <marketlib/include/BrokerModels.h>
#include <marketlib/include/enums.h>
#include "Appendix.hpp"
#include "settings.h"

using namespace std;
using namespace BrokerModels;
using namespace marketlib;
typedef instrument_descr_t Instrument;
// typedef order_t Order;


int main () 
{
    shared_ptr < ILogger > logger(new Logger);
    shared_ptr < ISettings > settings(new SerumSettings);
    shared_ptr < IPoolsRequester > pools(new SerumPoolsRequester(logger, settings));
    auto market = SerumMarket(PUBKEY, SECRETKEY, "https://solana-api.projectserum.com", pools, [](const string& a, const Instrument& b, const string& info){});

    cout << sizeof(InstructionLayoutOrderV3) << endl;
    Instrument instrument{"", "", "SOL/USDC", "SOL", "USDC" };
    order_t order;
    order.price = 3;
    order.original_qty = 0.1;
    order.side = order_side_t::os_Buy;


    market.send_new_order(instrument, order);
}