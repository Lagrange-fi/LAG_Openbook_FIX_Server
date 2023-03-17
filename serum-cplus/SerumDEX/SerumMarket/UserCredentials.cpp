#include "UserCredentials.hpp"
UserCredentials::UserCredentials(const string& client_id, const string& private_key, const string& endpoint) :
    _client_id(client_id), _private_key(private_key), _endpoint(endpoint) {}

UserCredentials::UserCredentials() : _client_id(), _private_key(), _endpoint()
{}

UserCredentials::~UserCredentials() 
{}	

UserCredentials::string UserCredentials::getID() const 
{
    return _client_id;
}

UserCredentials::string UserCredentials::getKey() const 
{
    return _private_key;
}

UserCredentials::string UserCredentials::getEndpoint() const
{
    return _endpoint;
}

void UserCredentials::setID(const string& client_id) 
{ 
    _client_id = client_id;
}

void UserCredentials::setKey(const string& private_key) 
{ 
    _private_key = private_key;
}

void UserCredentials::setEndpoint(const string& endpoint) 
{ 
    _endpoint = endpoint;
}

bool UserCredentials::Serialize(rapidjson::Writer<rapidjson::StringBuffer> * writer) const
{
    writer->StartObject();

    writer->String("client_id");
    writer->String(_client_id.c_str(), _client_id.size());

    writer->String("private_key");
    writer->String(_private_key.c_str(), _private_key.size());

    writer->String("endpoint");
    writer->String(_endpoint.c_str(), _endpoint.size());

    writer->EndObject();

    return true;
}

bool UserCredentials::Deserialize(const rapidjson::Value & obj)
{
    _client_id = obj["client_id"].GetString();
    _private_key = obj["private_key"].GetString();
    _endpoint = obj["endpoint"].GetString();

    return true;
}
