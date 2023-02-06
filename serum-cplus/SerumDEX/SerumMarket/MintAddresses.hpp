#pragma once 
#include <curl/curl.h>
#include <boost/json.hpp>
#include <sharedlib/include/ILogger.h>
#include <marketlib/include/market.h>
#include <sharedlib/include/HTTPClient.h>


class MintAdresses
{
private:
    typedef std::shared_ptr < ILogger > logger_ptr;
    typedef std::string string;

public:
    MintAdresses(logger_ptr logger_): logger(logger_) { loadAdresses();};
    ~MintAdresses() { _mint_addresses.clear(); };

    const string& get_address_for_token(const string& token)  {return _mint_addresses[token];}; 
private:
    logger_ptr logger;
    std::map<string, string> _mint_addresses;

    void loadAdresses()
    {
        string data_str;
        string address = "https://raw.githubusercontent.com/solana-labs/token-list/main/src/tokens/solana.tokenlist.json";
        try{
            data_str = HttpClient::request(
                "", 
                address, 
                HttpClient::HTTPMethod::GET
            );
        }
        catch(std::exception e) {
            throw string("Failed to make a request to " + address);
        }

        auto data = boost::json::parse(data_str).at("tokens").as_array();
        for(const auto& el : data) {
            _mint_addresses[el.at("symbol").as_string().c_str()] = el.at("address").as_string().c_str();
        }
    }
};