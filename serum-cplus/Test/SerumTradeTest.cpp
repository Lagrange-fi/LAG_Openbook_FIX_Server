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

    SerumTrade client(
        logger,
        settings,
        [&logger](const string& exch, broker_event event, const string& info) {}
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
            client.listen(instrument, "Cli_1", [&logger](const string& exch, const string& id, const execution_report_t& report) 
            {
                logger->Info("Cli_1");
                logger->Info(formatExecutionReport(exch, id, report).c_str());
            });
        } else if (cmd == "ulst") {
            client.unlisten(instrument, "Cli_1");
        } 
    }
}