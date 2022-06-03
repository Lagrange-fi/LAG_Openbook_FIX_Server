#include <iostream>
#include <string>
#include <atomic>

#include <SerumDEX/SerumTrade.h>
#include <marketlib/include/BrokerModels.h>
#include "Appendix.hpp"

using namespace std;
using namespace BrokerModels;
typedef marketlib::instrument_descr_t Instrument;



int main () {
    shared_ptr < ILogger > logger(new Logger);
    shared_ptr < ISettings > settings(new SerumSettings);
    shared_ptr < IBrokerApplication > application(new BrokerNullApplication(logger));

    SerumTrade client(
        logger,
        application,
        settings
    );

    Instrument instrument{"", "", "ETH/USDC", "ETH", "USDC" };

    client.start();
    
    this_thread::sleep_for(1s);

    while (true) {
        string cmd;
        cin >> cmd;
        if (cmd == "start") {
            client.start();
        } else if (cmd == "stop") {
            client.stop();
        } else if (cmd == "quit") {
            break;
        } else if (cmd == "lst") {
            client.listen(instrument);
        } else if (cmd == "ulst") {
            client.unlisten(instrument);
        } 
    }
}