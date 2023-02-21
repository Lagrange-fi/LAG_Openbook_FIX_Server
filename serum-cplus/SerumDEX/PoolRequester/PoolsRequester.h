#pragma once
#include "InstrumentsJSON.h"
#include <curl/curl.h>
#include <boost/json.hpp>
#include <sharedlib/include/IPoolsRequester.h>
#include <sharedlib/include/ILogger.h>
#include <sharedlib/include/ISettings.h>
#include <marketlib/include/market.h>


class PoolsRequester : public IPoolsRequester
{
private:
    // struct Pool {
    //     std::string name;
    //     std::string address;
    //     std::string program_id;
    //     bool deprecated;
    // };
    typedef marketlib::instrument_descr_t Instrument;
    typedef std::shared_ptr < ILogger > logger_ptr;
    typedef std::shared_ptr < ISettings > settings_ptr;
    typedef std::string string;

public:
    PoolsRequester(logger_ptr, settings_ptr, std::string path = "./data.json");
    void loadPools() override;
    std::list<Instrument> getPools() override;
    const Instrument& getPool(const Instrument&) override;
    ~PoolsRequester() override {};

private:
    logger_ptr _logger;
    settings_ptr _settings;
    InstrumentsJson _pools;
    std::string _path;
    // std::vector< Instrument > pools;
    std::list< Instrument > _pools_list;

    void loadPoolList();
    void savePoolsToJson();
    void loadPoolsFromJson();
    Instrument getPoolInfoFromServer(const Instrument&);
};