#pragma once 
#include <vector>
#include <string>
#include <curl/curl.h>
#include <iostream>
#include <boost/format.hpp>
// #include <cppcodec/base64_rfc4648.hpp>

class HttpClient 
{
private:
    typedef std::string string;
public:
    HttpClient();
    ~HttpClient();

    enum class HTTPMethod {
		GET,
		POST
	};

    static string request(const string& data, const string& target, HTTPMethod method, const std::vector<string>& headers = std::vector<string>()) {
        CURL* curl;
        CURLcode result;
        string response;
        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, target.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            if (method == HTTPMethod::POST) {
                curl_easy_setopt(curl, CURLOPT_POST, 1);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)data.length());
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            }

            struct curl_slist* headers_c = nullptr;
            if (headers.size()) {
                for (const auto& head : headers) {
                    headers_c = curl_slist_append(headers_c, head.c_str());
                }
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_c);
            }
            
            result = curl_easy_perform(curl);
            if (result != CURLE_OK) {
                throw std::string((boost::format(R"(Endpoint %1% request failed: %2%)") % target % response).str());
                response.clear();
            }
            curl_easy_cleanup(curl);
        } 
        else {
            // logger->Error("curl_easy_init error");
            throw std::string("curl_easy_init error");
        }
        return response;
    }

public:
    static size_t writeCallback(void* content, size_t size, size_t count, void* result) {
        ((string*)result)->append((char*)content, size * count);
        return size * count;
    }
};