#include "InstrumentsJSON.h"

bool InstrumentsJson::Deserialize(const std::string& s)
{
    rapidjson::Document doc;
    doc.Parse(s.c_str());

    if (!doc.IsArray())
        return false;

    for (rapidjson::Value::ConstValueIterator itr = doc.Begin(); itr != doc.End(); ++itr)
    {
        InstrumentJson p;
        p.Deserialize(*itr);
        _instruments.push_back(p);
    }

    return true;
}

bool InstrumentsJson::DeserializeFromFile(const std::string& path)
{
    std::string line;
    std::string result;
    std::ifstream in(path);
    if (in.is_open())
    {
        while (getline(in, line))
            result += line;
    }
    in.close();  

    return Deserialize(result);
}

bool InstrumentsJson::Serialize(rapidjson::Writer<rapidjson::StringBuffer>* writer) const
{
    writer->StartArray();

    for (std::list<InstrumentJson>::const_iterator it = _instruments.begin(); it != _instruments.end(); it++)
    {
        it->Serialize(writer);
    }
    writer->EndArray();

  return true;
}

bool InstrumentsJson::SerializeToFile(const std::string& path) const
{
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    Serialize(&writer);
    const std::string& serialized_instruments = buffer.GetString();

    std::ofstream  out(path);
    if (out.is_open())
        out << serialized_instruments;
    out.close();

    return true;
}


void InstrumentsJson::PushBackInstrument(const Instrument& instr)
{
    _instruments.push_back(InstrumentJson(instr));
}