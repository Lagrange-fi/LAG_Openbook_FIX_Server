#pragma once

#include <string>
#include <chrono>
#include <map>
#include <list>

namespace BrokerModels {
    // struct Instrument {
    //     std::string exchange;
    //     std::string symbol;
    //     std::string first;
    //     std::string second;
    //     std::string description;
    //     int decimals;
    // };


    struct MarketBook {
        std::chrono::time_point < std::chrono::system_clock > time;
        double bidPrice;
        double bidSize;
        double askPrice;
        double askSize;
        double lastPrice;
        double lastSize;
    };

    struct Market {
        std::map < double, double > bidSet;
        std::map < double, double > askSet;
    };

    struct MarketUpdate {
        double price;
        double volume;
        std::string entryId;
    };

    struct DepthSnapshot {
        std::list < MarketUpdate > bids;
        std::list < MarketUpdate > asks;
        std::chrono::time_point < std::chrono::system_clock > time;
        size_t sequence;
    };

};

