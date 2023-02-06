#pragma once
#include "InstrumentJSON.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <list>
#include <fstream>

class InstrumentsJson
{
private:
    typedef marketlib::instrument_descr_t Instrument;
public:	
    InstrumentsJson(): _instruments() {};
    ~InstrumentsJson() {};
    
    bool DeserializeFromFile(const std::string& path);
    bool SerializeToFile(const std::string& path)  const;
    std::list<InstrumentJson>& InstrumentsList() { return _instruments;}
    void PushBackInstrument(const Instrument& instr);
    
private:
    std::list<InstrumentJson> _instruments;

    bool Deserialize(const std::string& s) ;
    bool Serialize(rapidjson::Writer<rapidjson::StringBuffer>* writer)  const;
};