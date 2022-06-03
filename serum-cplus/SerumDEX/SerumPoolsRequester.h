#pragma once

#include <sharedlib/include/IPoolsRequester.h>
#include <sharedlib/include/ILogger.h>
#include <sharedlib/include/ISettings.h>
#include <marketlib/include/market.h>

class SerumPoolsRequester : public IPoolsRequester
{
private:
    typedef marketlib::instrument_descr_t Instrument;
    typedef std::shared_ptr < ILogger > logger_ptr;
    typedef std::shared_ptr < ISettings > settings_ptr;

public:
    SerumPoolsRequester(logger_ptr, settings_ptr);
    void loadPools() override;
    std::vector<Instrument> getPools() override;
    ~SerumPoolsRequester(){};

private:
    logger_ptr logger;
    settings_ptr settings;
    std::vector< Instrument > pools;

    std::vector< Instrument > getPoolsFromServer();
};