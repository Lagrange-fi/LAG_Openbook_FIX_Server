#include "PoolsRequester.h"
#include <boost/json/src.hpp>

using namespace std;

#define AS_STR(x) x.as_string().c_str()

PoolsRequester::PoolsRequester(logger_ptr logger, settings_ptr settings, std::string path):
	_logger(logger), _settings(settings), _pools(), _path(path), _pools_list()
{
	if (_path.size())
		loadPoolsFromJson();
	loadPoolList();
}

void PoolsRequester::loadPoolList()
{	
	string response;
	try{
		response = HttpClient::request(
			"", 
			"https://openserum.io/api/serum/markets.json",
			HttpClient::HTTPMethod::GET
		);
	}
	catch (string e){
		throw string("PoolsRequester::" + e);
	}

	auto parsed_data = boost::json::parse(response);
	for( auto pool : parsed_data.as_array()) {
		string symbol = AS_STR(pool.at("name"));

        Instrument instr;
        instr.engine = "OpenBook";
        instr.symbol=	symbol;
        instr.base_currency= symbol.substr(0, symbol.find("/"));
        instr.quote_currency= symbol.substr(symbol.find("/") + 1);
        instr.address= AS_STR(pool.at("address"));
        instr.program_id= AS_STR(pool.at("programId"));
        instr.deprecated= pool.at("deprecated").as_bool();
		_pools_list.push_back(instr);
    }
}

PoolsRequester::Instrument PoolsRequester::getPoolInfoFromServer(const Instrument& pool_info)
{
	string response;
	string url = "https://openserum.io/api/serum/market/" + pool_info.address;
	try{
		response = HttpClient::request(
			"", 
			url,
			HttpClient::HTTPMethod::GET
		);
	}
	catch (string e){
		throw string("PoolsRequester::" + e);
	}

	auto parsed_data = boost::json::parse(response);
	return Instrument{
		engine:	_settings->get(ISettings::Property::ExchangeName), 
		sec_id: "", 
		symbol: pool_info.symbol, 
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
	if (_pools.InstrumentsList().size()) {
		auto pool = std::find_if(_pools.InstrumentsList().begin(), _pools.InstrumentsList().end(), [&instrument](const InstrumentJson& i){ 
			return instrument.symbol == i.GetInstrument().symbol;
		});

		if (pool != std::end(_pools.InstrumentsList()))
			return *pool;
	}
	auto name = instrument.symbol;

	auto p = std::find_if(_pools_list.begin(), _pools_list.end(), [&name] (const Instrument& p) {
		return p.symbol == name;
	});

	if (p == std::end(_pools_list))
		throw std::string("Pool is not enabled");
	
	auto new_pool = getPoolInfoFromServer(*p);
	_pools.PushBackInstrument(new_pool);
	if (_path.size())
		savePoolsToJson();
	return _pools.InstrumentsList().back();
}

void PoolsRequester::loadPools() {
	loadPoolsFromJson();
}

void PoolsRequester::loadPoolsFromJson() 
{
	_pools.DeserializeFromFile(_path);
}

void PoolsRequester::savePoolsToJson()
{
	_pools.SerializeToFile(_path);
}

std::list<PoolsRequester::Instrument> PoolsRequester::getPools() 
{
	return _pools_list;
}