#include <iostream>
#include <string>
#include <atomic>

#include "SerumApp/SerumListener.h"
#include "SerumApp/SerumListener.cpp"
#include "BrokerLib/BrokerModels.h"
#include "Appendix.hpp"

using namespace std;
using namespace BrokerModels;



int main () {
    shared_ptr < ILogger > logger(new Logger);
    shared_ptr < ISettings > settings(new SerumSettings);
    shared_ptr < IBrokerApplication > application(new BrokerNullApplication(logger));

    SerumListener client(
        logger,
        application,
        settings
    );

    Instrument instrument{"Serum", "ETHUSDC", "ETH", "USDC" };

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