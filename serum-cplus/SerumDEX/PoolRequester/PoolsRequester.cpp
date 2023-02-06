#include "PoolsRequester.h"
#include <boost/json/src.hpp>

using namespace std;

#define AS_STR(x) x.as_string().c_str()

PoolsRequester::PoolsRequester(logger_ptr _logger, settings_ptr _settings, std::string _path):
	logger(_logger), settings(_settings), pools(), path(_path)
{
	loadPoolList(); 
	loadPoolsFromJson();
}

void PoolsRequester::loadPoolList()
{
	CURL *curl;
	CURLcode result;
	string response;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	try{
		curl = curl_easy_init();
		if (curl) {

			curl_easy_setopt(curl, CURLOPT_URL, "https://openserum.io/api/serum/markets.json");
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
		pools_list = std::list< Pool >();
	}

	pools_list = std::list< Pool >();
	auto parsed_data = boost::json::parse(response);
	for( auto pool : parsed_data.as_array()) {
		pools_list.push_back(
			PoolsRequester::Pool{
				name:	AS_STR(pool.at("name")), 
				address: AS_STR(pool.at("address")), 
				program_id: AS_STR(pool.at("programId")),
				deprecated: pool.at("deprecated").as_bool()
			}
		);
    }
}

PoolsRequester::Instrument PoolsRequester::getPoolInfoFromServer(const Pool& pool_info)
{
	CURL *curl;
	CURLcode result;
	string response;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	string url = "https://openserum.io/api/serum/market/" + pool_info.address;
	try{
		curl = curl_easy_init();
		if (curl) {

			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
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
		pools_list = std::list< Pool >();
	}

	auto parsed_data = boost::json::parse(response);
	return Instrument{
		engine:	settings->get(ISettings::Property::ExchangeName), 
		sec_id: "", 
		symbol: pool_info.name, 
		base_currency: 	AS_STR(parsed_data.at("baseSymbol")),
		quote_currency:	AS_STR(parsed_data.at("quoteSymbol")),
		address: 	pool_info.address,
		program_id:	pool_info.program_id,
		base_mint_address:	AS_STR(parsed_data.at("baseMint")),
		quote_mint_address: AS_STR(parsed_data.at("quoteMint")),
		tick_precision:	0,
		min_order_size:	0,
		deprecated:	pool_info.deprecated
	};
}

const PoolsRequester::Instrument& PoolsRequester::getPool(const Instrument& instrument) 
{
	if (pools.InstrumentsList().size()) {
		auto pool = std::find_if(pools.InstrumentsList().begin(), pools.InstrumentsList().end(), [&instrument](const InstrumentJson& i){ 
			return instrument.base_currency == i.GetInstrument().base_currency && 
				instrument.quote_currency == i.GetInstrument().quote_currency;
		});

		if (pool != std::end(pools.InstrumentsList()))
			return *pool;
	}
	auto name = instrument.base_currency + "/" + instrument.quote_currency;

	auto p = std::find_if(pools_list.begin(), pools_list.end(), [&name] (const Pool& p) {
		return p.name == name;
	});

	if (p == std::end(pools_list))
		throw "Pool is not enabled";
	
	auto new_pool = getPoolInfoFromServer(*p);
	pools.PushBackInstrument(new_pool);
	savePoolsToJson();
	return pools.InstrumentsList().back();
}

void PoolsRequester::loadPools() {
	loadPoolsFromJson();
}

void PoolsRequester::loadPoolsFromJson() 
{
	pools.DeserializeFromFile(path);
}

void PoolsRequester::savePoolsToJson()
{
	pools.SerializeToFile(path);
}

std::list<PoolsRequester::Instrument> PoolsRequester::getPools() 
{
	std::list<PoolsRequester::Instrument> newpools;
	
	for( const auto& p: pools.InstrumentsList() ) {
		newpools.push_back(p.GetInstrument());
	}
	return newpools;
}