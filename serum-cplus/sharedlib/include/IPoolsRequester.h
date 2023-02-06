#pragma once

#include <list>
#include <marketlib/include/market.h>

class IPoolsRequester 
{
private:
    typedef marketlib::instrument_descr_t Instrument;

protected:
    static size_t writeCallback(void* content, size_t size, size_t count, void* result) {
        ((std::string*)result)->append((char*)content, size * count);
        return size * count;
    }
    
public:
    virtual std::list<Instrument> getPools() = 0;
    virtual const Instrument& getPool(const Instrument&) = 0;
    virtual void loadPools() = 0;
    virtual ~IPoolsRequester() = default;
};

