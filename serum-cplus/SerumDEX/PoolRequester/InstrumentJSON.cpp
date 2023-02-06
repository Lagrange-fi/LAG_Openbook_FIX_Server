#include "InstrumentJSON.h"

bool InstrumentJson::Serialize(rapidjson::Writer<rapidjson::StringBuffer> * writer) const
{
    writer->StartObject();

    writer->String("engine");
    writer->String(engine.c_str(), engine.size());

    writer->String("sec_id");
    writer->String(sec_id.c_str(), sec_id.size());

    writer->String("symbol");
    writer->String(symbol.c_str(), symbol.size());

    writer->String("base_currency");
    writer->String(base_currency.c_str(), base_currency.size());

    writer->String("quote_currency");
    writer->String(quote_currency.c_str(), quote_currency.size());

    writer->String("address");
    writer->String(address.c_str(), address.size());

    writer->String("program_id");
    writer->String(program_id.c_str(), program_id.size());

    writer->String("base_mint_address");
    writer->String(base_mint_address.c_str(), base_mint_address.size());

    writer->String("quote_mint_address");
    writer->String(quote_mint_address.c_str(), quote_mint_address.size());

    writer->String("tick_precision");
    writer->Int(tick_precision);

    writer->String("min_order_size");
    writer->Double(min_order_size);

    writer->String("deprecated");
    writer->Bool(deprecated);

    writer->EndObject();

    return true;
}

bool InstrumentJson::Deserialize(const rapidjson::Value & obj)
{
    engine = obj["engine"].GetString();
    sec_id = obj["sec_id"].GetString();
    symbol = obj["symbol"].GetString();
    base_currency = obj["base_currency"].GetString();
    quote_currency = obj["quote_currency"].GetString();
    address = obj["address"].GetString();
    program_id = obj["program_id"].GetString();
    base_mint_address = obj["base_mint_address"].GetString();
    quote_mint_address = obj["quote_mint_address"].GetString();
    tick_precision = obj["tick_precision"].GetInt();
    min_order_size = obj["min_order_size"].GetDouble();
    deprecated = obj["deprecated"].GetBool();

    return true;
}
