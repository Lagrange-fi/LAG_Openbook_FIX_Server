#include <iostream>
#include <string>
#include <atomic>
#include <any>

#include <SerumDEX/SerumMD.h>
#include <SerumDEX/PoolRequester/PoolsRequester.h>
// #include <SerumDEX/SerumAdapter.h>
#include <marketlib/include/BrokerModels.h>
#include <marketlib/include/enums.h>
#include "Appendix.hpp"

using namespace std;
using namespace BrokerModels;
using namespace marketlib;
typedef marketlib::instrument_descr_t Instrument;


int main () {
    shared_ptr < ILogger > logger(new Logger);
    shared_ptr < ISettings > settings(new SerumSettings);
    shared_ptr < IPoolsRequester > pools(new PoolsRequester(logger, settings));

    SerumMD client(
        logger,
        settings,
        pools,
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
        } else if (cmd == "st") {
            client.subscribe(instrument, market_depth_t::top, "Cli_1",  [&logger](const string& exch, const string& symbol, const std::any& data)
            {
                logger->Info("Cli_1");
                logger->Info(formatTopInfo(exch, symbol, std::any_cast<BrokerModels::MarketBook>(data)).c_str());
            });
        } else if (cmd == "ut") {
            client.unsubscribe(instrument, market_depth_t::top, "Cli_1");
        } else if (cmd == "st2") {
            client.subscribe(instrument, market_depth_t::top, "Cli_2", [&logger](const string& exch, const string& symbol, const std::any& data)
            {
                logger->Info("Cli_2");
                logger->Info(formatTopInfo(exch, symbol, std::any_cast<BrokerModels::MarketBook>(data)).c_str());
            });
        } else if (cmd == "ut2") {
            client.unsubscribe(instrument, market_depth_t::top, "Cli_2");
        }else if (cmd == "sd") {
            client.subscribe(instrument, market_depth_t::full, "Cli_1", [&logger](const string& exch, const string& symbol, const std::any& data)
            {
                logger->Info("Cli_1");
                logger->Info(formatDepthInfo(exch,  symbol, std::any_cast<BrokerModels::DepthSnapshot>(data)).c_str());
            });
        } else if (cmd == "ud") {
            client.unsubscribe(instrument, market_depth_t::full, "Cli_1");
        } else if (cmd == "inst") {
            auto instruments = client.getInstruments();
            cout << "Instr count: " << instruments.size() << endl;

            /*
             {
              "name": string,
              "baseMintAddress": string,
              "quoteMintAddress": string,
              "version": number,
              "address": string,
              "programId": string,
              "baseCurrency": string,
              "quoteCurrency": string,
              "tickSize": number,
              "minOrderSize": number,
              "deprecated": boolean
            }[]
            sample response
            [
              {
                "name": "BTC/USDC",
                "baseCurrency": "BTC",
                "quoteCurrency": "USDC",
                "version": 3,
                "address": "A8YFbxQYFVqKZaoYJLLUVcQiWP7G2MeEgW5wsAQgMvFw",
                "programId": "9xQeWvG816bUx9EPjHmaT23yvVM2ZWbrrpZb9PusVFin",
                "baseMintAddress": "9n4nbM75f5Ui33ZbPYXn59EwSgE8CGsHtAeTH5YFeJ9E",
                "quoteMintAddress": "EPjFWdd5AufqSSqeM2qN1xzybapC8G4wEGGkZwyTDt1v",
                "tickSize": 0.1,
                "minOrderSize": 0.0001,
                "deprecated": false
              }
            ]
             */
            for(auto instr : instruments) {
                cout 
                // "Exch: "
                // << instr.engine
                << "  Market: "
                << instr.symbol
                << " base_curency: "
                << instr.base_currency
                << " quote_curency: "
                << instr.quote_currency
                // <<"  Currency: "
                // << instr.quote_currency
                // << "  sec_id: "
                // << instr.sec_id
                // << "  precision: "
                // << instr.tick_precision
                << " address "
                << instr.address
                // << " base_min_addr "
                // << instr.base_mint_address
                // << " quote_mint_addr "
                // << instr.quote_mint_address
                // << "min_order_size "
                // << instr.min_order_size
                << endl;

            }

            /*
                const std::string   engine;
                std::string     sec_id;
                std::string     symbol;
                std::string     base_currency;
                std::string     quote_currency;
                std::string     address;
                std::string     program_id;
                std::string     base_mint_address;
                std::string     quote_mint_address;
                int             tick_precision;
                double          min_order_size;
                bool            deprecated;
             */
        }
    }
}

