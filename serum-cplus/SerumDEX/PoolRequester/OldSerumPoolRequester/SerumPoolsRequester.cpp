#include "SerumPoolsRequester.h"
#include <boost/json/src.hpp>

using namespace std;

#define AS_STR(x) x.as_string().c_str()

SerumPoolsRequester::SerumPoolsRequester(logger_ptr _logger, settings_ptr _settings):
	logger(_logger), settings(_settings), pools() 
{
	// loadPools();
}

std::vector<SerumPoolsRequester::Instrument> SerumPoolsRequester::getPoolsFromServer()
{
    CURL *curl;
	CURLcode result;
	string response;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	try{
		curl = curl_easy_init();
		if (curl) {

			curl_easy_setopt(curl, CURLOPT_URL, "https://api.serum-vial.dev/v1/markets");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

			result = curl_easy_perform(curl);
			// Error check
			if (result != CURLE_OK) {
				logger->Error("curl_easy_perform() error");
				response.clear();
				throw ;
			}

			curl_easy_cleanup(curl);
		} else {
			logger->Error("curl_easy_init() error");
			throw ;
		}
	}
	catch(int e) {
		return std::vector< Instrument >();
	}
	
	auto get_precision = [](double a) -> int {
		return 18 - to_string(a * 10e16).find('.');
	};

	auto get_num = [](boost::json::value a) {
		double b = 0;
		try {
			b = a.as_double();
		}
		catch ( std::exception e ) {
			b = a.as_int64();
		} 

		return b;
	};

	auto instruments = std::vector< Instrument > ();
    auto parsed_data = boost::json::parse(response);
    for( auto pool : parsed_data.as_array()) {
		instruments.push_back(
			Instrument{
				engine:	settings->get(ISettings::Property::ExchangeName), 
				sec_id: "", 
				symbol: AS_STR(pool.at("name")), 
				base_currency: 	AS_STR(pool.at("baseCurrency")),
				quote_currency:	AS_STR(pool.at("quoteCurrency")),
				address: 	AS_STR(pool.at("address")),
				program_id:	AS_STR(pool.at("programId")),
				base_mint_address:	AS_STR(pool.at("baseMintAddress")),
				quote_mint_address: AS_STR(pool.at("quoteMintAddress")),
				tick_precision:	get_precision(get_num(pool.at("tickSize"))),
				min_order_size:	get_num(pool.at("minOrderSize")),
				deprecated:	pool.at("deprecated").as_bool()
			}
		);
    }
    return instruments;
}


void SerumPoolsRequester::loadPools() {
	this->pools = getPoolsFromServer();
}

std::vector<SerumPoolsRequester::Instrument> SerumPoolsRequester::getPools() {
	return this->pools;
}