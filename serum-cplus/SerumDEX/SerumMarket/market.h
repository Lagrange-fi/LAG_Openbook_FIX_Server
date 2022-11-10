#pragma once 

#include "enums.h"
#include "structs.h"

#include "constants.h"
#include "sol_sdk/key.h"
#include "sol_sdk/types.h"
#include "sol_sdk/cpi.h"
#include "sol_sdk/entrypoint.h"

#include <boost/json.hpp>
#include <boost/format.hpp>
#include <algorithm>
#include <string>
#include <map>

#include <marketlib/include/enums.h>
#include <marketlib/include/market.h>
#include <sharedlib/include/IPoolsRequester.h>
#include <sharedlib/include/HTTPClient.h>
#include <sharedlib/include/IMarket.h>

#include <functional>
#include "transaction.h"



class SerumMarket : IMarket
{
private:
    typedef std::string string;
    typedef std::shared_ptr < IPoolsRequester > pools_ptr;
    typedef marketlib::order_t Order;
    typedef marketlib::instrument_descr_t Instrument;
    typedef std::function <void(const string&, const Instrument&, const string&)> Callback;

    struct MarketChannel
    {
        string base;
        string quote;
		Instrument instr;
        SolPubkey market_address;
        SolPubkey payer_sell;
        SolPubkey payer_buy;
        SolPubkey open_order_account;
        MarketLayout parsed_market;
        uint64_t base_spl_token_multiplier;
        uint64_t quote_spl_token_multiplier;
    };

    using MarketChannels = boost::multi_index::multi_index_container<
        MarketChannel,
        boost::multi_index::indexed_by<
            boost::multi_index::hashed_unique<
                boost::multi_index::tag<struct MarketChannelsByPool>,
                boost::multi_index::composite_key<
                    MarketChannel,
                    boost::multi_index::member<MarketChannel, decltype(MarketChannel::base), &MarketChannel::base >,
					boost::multi_index::member<MarketChannel, decltype(MarketChannel::quote), &MarketChannel::quote >
                >
            >
        >
    >;
    // typedef std::shared_ptr < ILogger > logger_ptr;

    string pubkey_;
    SolPubkey decoded_pubkey_;
    string secretkey_;
    SolKeyPair decoded_secretkey_;
    string http_address_;
    pools_ptr pools_;
    Callback callback_;
    MarketChannels markets_info;
    std::map<string, string> mint_addresses_;

    
    void place_order(
        const MarketChannel&,
        OrderType,
        Side,
        double,
        double,
        uint64_t
    );

    // create market info

    void get_mint_addresses();
    MarketChannel create_market_info(const Instrument&);
    MarketLayout get_market_layout(const string&);
    string get_account_info(const string&);
    string get_latest_blockhash();
    string get_token_account_by_owner(const string&, const string&);
    string get_token_program_account(const string&, const string&, const string&);
    uint8_t get_mint_decimals(const string&);

    // translating a data structure into bytes 
    std::function<void(uint8_t *, const void *, size_t)> serialize = std::memcpy;


    // translating bytes into a data structure
    std::function<void(void *, const uint8_t *, size_t)> deserialize = std::memcpy;

    void new_order_v3(const NewOrderV3Params&, SolInstruction&);
    void new_cancel_order_v2(const CancelOrderV2Params&, SolInstruction&);

    // precision
    uint64_t price_number_to_lots(long double price, const MarketChannel& info);
    uint64_t base_size_number_to_lots(long double price, const MarketChannel& info);
    string send_transaction(Transaction& transaction, const std::vector<SolKeyPair>&);

public:
    SerumMarket(const string&, const string&, const string&, pools_ptr, Callback);
    SerumMarket(const SerumMarket&) {};

    void send_new_order(const Instrument&, const Order&) override;
    void cancel_order(const Instrument&, const Order&) override;

    ~SerumMarket();
};