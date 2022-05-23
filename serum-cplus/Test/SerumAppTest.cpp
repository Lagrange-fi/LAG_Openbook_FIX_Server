#include <iostream>
#include <string>
#include <atomic>

#include "SerumApp/SerumApp.h"
#include "SerumApp/SerumApp.cpp"
#include "BrokerLib/BrokerModels.h"
#include "Appendix.hpp"

using namespace std;
using namespace BrokerModels;



int main () {
    shared_ptr < ILogger > logger(new Logger);
    shared_ptr < ISettings > settings(new SerumSettings);
    shared_ptr < IBrokerApplication > application(new BrokerNullApplication(logger));

    SerumApp client(
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
        } else if (cmd == "st") {
            client.subscribe(instrument, IBrokerClient::SubscriptionModel::TopBook);
        } else if (cmd == "ut") {
            client.unsubscribe(instrument, IBrokerClient::SubscriptionModel::TopBook);
        } else if (cmd == "sd") {
            client.subscribe(instrument, IBrokerClient::SubscriptionModel::FullBook);
        } else if (cmd == "ud") {
            client.unsubscribe(instrument, IBrokerClient::SubscriptionModel::FullBook);
        }
    }
}