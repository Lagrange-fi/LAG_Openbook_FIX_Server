#pragma once

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <marketlib/include/market.h>

class InstrumentJson : public marketlib::instrument_descr_t
{
private:
    typedef marketlib::instrument_descr_t Instrument;
public:
    
    // InstrumentJson() : Instrument("default_engine") {};
    InstrumentJson(const std::string&  _engine = "", 
        const std::string&  _sec_id = "",
        const std::string&  _symbol = "",
        const std::string&  _base_currency = "",
        const std::string&  _quote_currency = "",
        const std::string&  _address = "",
        const std::string&  _program_id = "",
        const std::string&  _base_mint_address = "",
        const std::string&  _quote_mint_address = "",
        int                 _tick_precision = 0,
        double              _min_order_size = 0,
        bool                _deprecated = true)
        {
            engine = _engine;
            sec_id = _sec_id;
            symbol = _symbol;
            base_currency = _base_currency;
            quote_currency = _quote_currency;
            address = _address;
            program_id = _program_id;
            base_mint_address = _base_mint_address;
            quote_mint_address = _quote_mint_address;
            tick_precision = _tick_precision;
            min_order_size = _min_order_size;
            deprecated = _deprecated;
        }

    InstrumentJson(const Instrument& instr)
    {
        engine = instr.engine;
        sec_id = instr.sec_id;
        symbol = instr.symbol;
        base_currency = instr.base_currency;
        quote_currency = instr.quote_currency;
        address = instr.address;
        program_id = instr.program_id;
        base_mint_address = instr.base_mint_address;
        quote_mint_address = instr.quote_mint_address;
        tick_precision = instr.tick_precision;
        min_order_size = instr.min_order_size;
        deprecated = instr.deprecated;
    }	
    ~InstrumentJson() {};			

    bool Deserialize(const rapidjson::Value& obj);
    bool Serialize(rapidjson::Writer<rapidjson::StringBuffer>* writer) const;
    const Instrument& GetInstrument() const { return *this; };
};	