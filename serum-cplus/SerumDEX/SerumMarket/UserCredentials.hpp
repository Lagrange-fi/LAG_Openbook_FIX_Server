#pragma once

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <string>

class UserCredentials
{
private:
    typedef std::string string;

    string _client_id;
    string _private_key;
    string _endpoint;
public:
    UserCredentials(const string& client_id, const string& private_key, const string& endpoint);
    UserCredentials();
    ~UserCredentials();			

    string getID() const;
    string getKey() const;
    string getEndpoint() const;
    void setID(const string& client_id);
    void setKey(const string& private_key);
    void setEndpoint(const string& endpoint);
    
    bool Deserialize(const rapidjson::Value& obj);
    bool Serialize(rapidjson::Writer<rapidjson::StringBuffer>* writer) const;
    // const Instrument& GetInstrument() const { return *this; };
};