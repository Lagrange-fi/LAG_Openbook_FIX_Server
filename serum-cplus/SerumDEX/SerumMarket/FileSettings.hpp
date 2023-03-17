#pragma once

#include <string>
#include <sharedlib/include/IMarketSettings.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include "UserCredentials.hpp"
#include <fstream>

class FileSettings : public IMarketSettings
{
private:
    typedef std::string string;

    UserCredentials _credentials;
    string _path;
public:
    FileSettings(const string& client_Id, const string& private_key, const string& endpoint, const string& path);
    FileSettings(const string& path);
    ~FileSettings();
    bool DeserializeFromFile(const string& path);
    bool SerializeToFile(const string& path)  const;

    string get(Property) const override;

private:
    bool Deserialize(const string& s) ;
    bool Serialize(rapidjson::Writer<rapidjson::StringBuffer>* writer)  const;
};