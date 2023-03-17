#include "FileSettings.hpp"

FileSettings::FileSettings(const string& client_Id, const string& private_key, const string& endpoint ,const string& path) : 
    _credentials(client_Id, private_key, endpoint), _path(path) 
{
    
}

FileSettings::FileSettings(const string& path) : _path(path), _credentials()
{
    DeserializeFromFile(_path);
}

FileSettings::~FileSettings()
{
    SerializeToFile(_path);
}

FileSettings::string FileSettings::get(Property tag) const
{
    if (tag == Property::ClientId)
        return _credentials.getID();
    else if (tag == Property::PrivateKey)
        return _credentials.getKey();
    else if (tag == Property::Endpoint)
        return _credentials.getEndpoint();
    else
        return "";
}

bool FileSettings::Deserialize(const string& s)
{
    rapidjson::Document doc;
    doc.Parse(s.c_str());

    // if (!doc.IsArray())
    //     return false;
    return _credentials.Deserialize(doc);
}

bool FileSettings::Serialize(rapidjson::Writer<rapidjson::StringBuffer>* writer) const
{
  return _credentials.Serialize(writer);;
}

bool FileSettings::DeserializeFromFile(const string& path)
{
    string line;
    string result;
    std::ifstream in(path);
    if (in.is_open())
        while (getline(in, line))
            result += line;
    else
        throw string("Settings fail does not exist");
    in.close();  

    return Deserialize(result);
}

bool FileSettings::SerializeToFile(const string& path)  const
{
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    auto serialize_result = Serialize(&writer);
    if (!serialize_result)
        return serialize_result;
    const string& serialized_instruments = buffer.GetString();

    std::ofstream  out(path);
    if (out.is_open())
        out << serialized_instruments;
    out.close();

    return true;
}