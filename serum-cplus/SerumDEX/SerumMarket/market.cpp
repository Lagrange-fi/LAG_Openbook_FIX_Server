#include "market.h"
#include "instruments.h"
#include <base64/base64.h>
#include "sysvar.h"
#include <iostream>
#include <cmath>
#include "transaction.h"
// #include <nacl/crypto_stream.h>

SerumMarket::SerumMarket(const string& pubkey, const string& secretkey, const string& http_address, pools_ptr pools, Callback callback) 
: pubkey_(pubkey), secretkey_(secretkey), http_address_(http_address), pools_(pools), callback_(callback),
  decoded_pubkey_(base58str_to_pubkey(pubkey)), decoded_secretkey_(base58str_to_keypair(secretkey))
{
    get_mint_addresses();
}

SerumMarket::~SerumMarket()
{
    mint_addresses_.clear();
}


// // 'url': 'https://solana-api.projectserum.com', 
// // 'headers': {'Content-Type': 'application/json'}, 
// // 'data': '{"jsonrpc": "2.0", "id": 1, "method": "getAccountInfo", "params": ["9wFFyRfZBsuAha4YcuxcXLKwMxJR43S7fPfQLusDBzvT", {"encoding": "base64", "commitment": "finalized"}]}'}
// std::string Market::get_account_info(const string& key)
// {

// }

void SerumMarket::place_order(
    const MarketChannel& info,
    OrderType order_type,
    Side side,
    double limit_price,
    double max_quantity,
    uint64_t client_id = 0 
)
{
    struct NewOrderV3Params order_instruction = 
    {
        market: info.market_address,
        open_orders: info.open_order_account,
        payer: side == Side::BUY ? info.payer_buy : info.payer_sell,
        owner: decoded_pubkey_,
        request_queue: info.parsed_market.request_queue,
        event_queue: info.parsed_market.event_queue,
        bids: info.parsed_market.bids,
        asks: info.parsed_market.asks,
        base_vault: info.parsed_market.base_vault,
        quote_vault: info.parsed_market.quote_vault,
        side: side,
        // // TODO: to make functions to recalculate coins
        limit_price: price_number_to_lots(limit_price, info),
        max_base_quantity: base_size_number_to_lots(max_quantity, info), 
        max_quote_quantity: base_size_number_to_lots(max_quantity, info) 
        * info.parsed_market.quote_lot_size
        * price_number_to_lots(limit_price, info),
        order_type: order_type,
        self_trade_behavior: SelfTradeBehavior::DECREMENT_TAKE,
        limit: 65535,
        client_id: client_id,
        program_id: base58str_to_pubkey("9xQeWvG816bUx9EPjHmaT23yvVM2ZWbrrpZb9PusVFin")
    };

    SolInstruction instruction;
    new_order_v3(order_instruction, instruction);

    Transaction txn;
    txn.add_instruction(instruction);
    std::vector<SolKeyPair> signers;
    signers.push_back(decoded_secretkey_);

    auto res = send_transaction(txn, signers);
}

void SerumMarket::new_cancel_order_v2(const CancelOrderV2Params& params, SolInstruction& instruction)
{
    instruction.program_id = params.program_id;
    instruction.accounts = new SolAccountMeta[6] {
        SolAccountMeta { pubkey: params.market, is_writable: false, is_signer: false },
        SolAccountMeta { pubkey: params.bids, is_writable: true, is_signer: false },
        SolAccountMeta { pubkey: params.asks, is_writable: true, is_signer: false },
        SolAccountMeta { pubkey: params.open_orders, is_writable: true, is_signer: false },
        SolAccountMeta { pubkey: params.owner, is_writable: false, is_signer: true },
        SolAccountMeta { pubkey: params.event_queue, is_writable: true, is_signer: false }
    };
    instruction.account_len = 6;

    CancelOrderV2 cancel_order_v2;
    cancel_order_v2.side = params.side;
    memcpy(cancel_order_v2.order_id, params.order_id, 16);

    auto ord_layout = InstructionLayoutCancelOrderV2 {
        0,
        11,
        cancel_order_v2
    };

    serialize( 
        instruction.data,
        &ord_layout,  
        sizeof(InstructionLayoutCancelOrderV2)
    );

    instruction.data_len = sizeof(InstructionLayoutCancelOrderV2);
}

void SerumMarket::cancel_order(const Instrument& instrument, const Order& order)
{
    auto market_info = markets_info.get<MarketChannelsByPool>()
		.find(boost::make_tuple(
			instrument.base_currency,
			instrument.quote_currency
		));

    if (market_info == markets_info.end()) {
        auto pls = pools_->getPools();
        auto pool = *std::find_if(pls.begin(), pls.end(), [&instrument](const Instrument& i){ 
                return instrument.base_currency == i.base_currency && 
                    instrument.quote_currency == i.quote_currency;
            });

        markets_info.insert(create_market_info(pool));
        market_info = markets_info.begin();
    }

    auto info = *market_info;



    CancelOrderV2Params params {
        market: info.market_address,
        bids: info.parsed_market.bids,
        asks: info.parsed_market.asks,
        event_queue: info.parsed_market.event_queue,
        open_orders: info.open_order_account,
        owner: decoded_pubkey_,
        side: marketlib::order_side_t::os_Buy ? Side::BUY : Side::SELL,
        // order_id: 7849659000233099250,
        // // open_orders_slot: 0,
        // program_id: base58str_to_pubkey("9xQeWvG816bUx9EPjHmaT23yvVM2ZWbrrpZb9PusVFin")
    };

    params.program_id = base58str_to_pubkey("9xQeWvG816bUx9EPjHmaT23yvVM2ZWbrrpZb9PusVFin");

    SolInstruction instruction;
    new_cancel_order_v2(params, instruction);
    Transaction txn;
    txn.add_instruction(instruction);
    txn.set_recent_blockhash(get_latest_blockhash());
    txn.get_message_for_sign();
    txn.sign(decoded_secretkey_);
}

void SerumMarket::send_new_order(const Instrument& instrument, const Order& order) 
{
    auto market_info = markets_info.get<MarketChannelsByPool>()
		.find(boost::make_tuple(
			instrument.base_currency,
			instrument.quote_currency
		));

    if (market_info == markets_info.end()) {
        auto pls = pools_->getPools();
        auto pool = *std::find_if(pls.begin(), pls.end(), [&instrument](const Instrument& i){ 
                return instrument.base_currency == i.base_currency && 
                    instrument.quote_currency == i.quote_currency;
            });

        markets_info.insert(create_market_info(pool));
        market_info = markets_info.begin();
    }

    // auto payer = order.side == marketlib::order_side_t::os_Buy ? market_info->payer_buy : market_info->payer_sell;

    place_order(
        *market_info,
        OrderType::LIMIT,
        order.side == marketlib::order_side_t::os_Buy ? Side::BUY : Side::SELL,
        order.price,
        order.original_qty,
        // TODO Cli_id
        7849659000233099250
    );
}

SerumMarket::MarketChannel SerumMarket::create_market_info(const Instrument& instr)
{
    auto payer_buy = get_token_account_by_owner(pubkey_, instr.quote_mint_address);

    // TODO place_order_open_order_account if open_order_account not exist
    auto open_order_account = get_token_program_account(
        "9xQeWvG816bUx9EPjHmaT23yvVM2ZWbrrpZb9PusVFin",
        instr.address,
        pubkey_
    );

    return MarketChannel {
        base: instr.base_currency,
        quote: instr.quote_currency,
        instr: instr,
        market_address: base58str_to_pubkey(instr.address),
        payer_sell: base58str_to_pubkey(pubkey_),
        payer_buy: base58str_to_pubkey(payer_buy),
        open_order_account: base58str_to_pubkey(open_order_account),
        parsed_market: get_market_layout(instr.address),
        base_spl_token_multiplier: static_cast<uint64_t>(pow(10, get_mint_decimals(instr.base_mint_address))),
        quote_spl_token_multiplier: static_cast<uint64_t>(pow(10, get_mint_decimals(instr.quote_mint_address)))
    };
}

MarketLayout SerumMarket::get_market_layout(const string& market_address)
{
    auto account_data = base64_decode(
        string(boost::json::parse(get_account_info(market_address)).at("result").at("value").at("data").as_array()[0].as_string().c_str())
    );
    // auto account_data = base64_decode(get_account_info(market_address));
    // ignore the first 5 bytes (the word 'serum') and the last 7 bytes (the word 'padding')
    // example of data after decoding:
    // "serum\003\000\000\000\000\000\000\000\204\302\373\030\256\326\031\365Fc&S\357\006\002\237\002\250d\277
    // \070)\206q\201\273 \337\035q\\0\001\000\000\000\000\000\000\000\006\233\210W\376\253\201\204\373h\177cF
    // \030\300\065\332\304\071\334\032\353;U\230\240\360\000\000\000\000\001\306\372z\363\276ۭ:=e\363j\253\311
    // t1\261\273\344\302\322\366\340\344|\246\002\003E/]a\037&\365\360F\034@\020\275\\\310\312pfݥ\204\246\356q
    // y4\306w\255\364\302_\275\025j-\000u@@@\303\001\000\000\000\000\000\000\000\000\000j\343\355\062z\017\210I
    // \247r\224\035\227\005\017:n\214\270\335:\274\333\024p\210|\202\265M?6\244\200\322'\214\020\000\000\027g\245
    // \005\364\000\000\000d\000\000\000\000\000\000\000\215\374\310\365{\017\316{ufu\232\366\357gT\250\065*&\303
    // \344Ik\315'\333~\326)\226\342@ \211FS\317\335\372{~`\311fh'6\242\333\017\203\205d\222[\021\az!\340\066ח\000\364&\341
    // n\270\317\003\021\221u\371\200QCD\225\\\343p\347e\224\017<)C\225E\373E\251\246\337\321\\Pw\005\371\063\233\225<\032M\375
    // \274\234\301\206\335/b\337H\251X\004^*vRY\000\341\365\005\000\000\000\000d\000\000\000\000\000\000\000\000\000\000
    // \000\000\000\000\000`\330n\217\020\000\000\000padding"
    auto buffer_size = account_data.size() - 5 - 7;
    char buffer[buffer_size];
    account_data.copy(buffer, buffer_size, 5);
    MarketLayout tmp; 
    deserialize((void*)&tmp, (uint8_t*)buffer, buffer_size);
    return tmp;
}

uint8_t SerumMarket::get_mint_decimals(const string& mint_address)
{
    if (mint_address == WRAPPED_SOL_MINT) 
        return 9;

    return (uint8_t)boost::json::parse(get_account_info(mint_address))
        .at("result")
        .at("value")
        .at("data")
        .at("parsed")
        .at("info")
        .at("decimals").as_int64();
}

void SerumMarket::get_mint_addresses()
{
    string data_str;
    try{
        data_str = HttpClient::request(
            "", 
            "https://raw.githubusercontent.com/project-serum/serum-ts/master/packages/serum/src/token-mints.json", 
            HttpClient::HTTPMethod::GET
        );
    }
    catch(std::exception e) {

    }

    auto data = boost::json::parse(data_str).as_array();
    for(const auto& el : data) {
        mint_addresses_[el.at("name").as_string().c_str()] = el.at("address").as_string().c_str();
    }
}

std::string SerumMarket::get_latest_blockhash()
{
    string data_str;
    try{
        data_str = HttpClient::request(
            R"({"jsonrpc": "2.0", "id": 1, "method": "getLatestBlockhash", "params": [{"commitment": "finalized"}]})", 
            http_address_, 
            HttpClient::HTTPMethod::POST,
            std::vector<string>({"Content-Type: application/json"})
        );
    }
    catch(std::exception e) {

        return "";
    }

    return boost::json::parse(data_str).at("result").at("value").at("blockhash").as_string().c_str();
}

std::string SerumMarket::get_token_account_by_owner(const string& owner_pubkey, const string& token_address) 
{
    string data_str;
    try{
        data_str = HttpClient::request(
            (boost::format(R"({
                "jsonrpc": "2.0", 
                "id": 1, 
                "method": "getTokenAccountsByOwner", 
                "params": [
                    "%1%", 
                    {"mint": "%2%"}, 
                    {"commitment": "finalized", "encoding": "base64"}
                ]
            })") % owner_pubkey % token_address).str(), 
            http_address_, 
            HttpClient::HTTPMethod::POST,
            std::vector<string>({"Content-Type: application/json"})
        );
    }
    catch(std::exception e) {

        return "";
    }

    return boost::json::parse(data_str).at("result").at("value").as_array()[0].at("pubkey").as_string().c_str();
}


// '{"jsonrpc": "2.0", "id": 3, "method": "getProgramAccounts", "params": ["9xQeWvG816bUx9EPjHmaT23yvVM2ZWbrrpZb9PusVFin", {"filters": [{"memcmp": {"offset": 13, "bytes": "9wFFyRfZBsuAha4YcuxcXLKwMxJR43S7fPfQLusDBzvT"}}, {"memcmp": {"offset": 45, "bytes": "GKvwL3FmQRHuB9mcZ3WuqTuVjbGDzdW51ec8fYdeHae1"}}, {"dataSize": 3228}], "encoding": "base64", "commitment": "recent"}]}'

std::string SerumMarket::get_token_program_account(const string& market_key, const string& pool_key, const string& pubkey_owner)
{
    string data_str;
    try{
        data_str = HttpClient::request(
            (boost::format(R"({
                "jsonrpc": "2.0", 
                "id": 1, 
                "method": "getProgramAccounts", 
                "params": [
                    "%1%", 
                    {"filters": [
                        {"memcmp": {"offset": 13, "bytes": "%2%"}}, 
                        {"memcmp": {"offset": 45, "bytes": "%3%"}}, 
                        {"dataSize": 3228}
                    ], 
                    "encoding": "base64", 
                    "commitment": "recent"}
                ]
            })") % market_key % pool_key % pubkey_owner).str(), 
            http_address_, 
            HttpClient::HTTPMethod::POST,
            std::vector<string>({"Content-Type: application/json"})
        );
    }
    catch(std::exception e) {

        return "";
    }

    return boost::json::parse(data_str).at("result").as_array()[0].at("pubkey").as_string().c_str();
}

//'{"jsonrpc": "2.0", "id": 1, "method": "getAccountInfo", "params": ["9wFFyRfZBsuAha4YcuxcXLKwMxJR43S7fPfQLusDBzvT", {"encoding": "base64", "commitment": "finalized"}]}'
std::string SerumMarket::get_account_info(const string& account)
{
    string data_str;
    try{
        data_str = HttpClient::request(
            (boost::format(R"({
                "jsonrpc": "2.0", 
                "id": 1, 
                "method": "getAccountInfo", 
                "params": [
                    "%1%", 
                    {"encoding": "jsonParsed", "commitment": "finalized"}
                ]
            })") % account).str(), 
            http_address_, 
            HttpClient::HTTPMethod::POST,
            std::vector<string>({"Content-Type: application/json"})
        );
    }
    catch(std::exception e) {

        return "";
    }

    return data_str;
}

/*

{
    "jsonrpc": "2.0", 
    "id": 5, 
    "method": "sendTransaction", 
    "params": [
        "AVkDoh1wcZqDMc1icCPVrg59yZdxfZR2P5bprZrW/jflXzRsWaOx0pHMGT6sUs/And7BShalLRcy/w3OVgB0LQcBAAMNisYu1t7X1daKhI9tQrdE/a5H4usgBan8UnExaoU/M/8A9CbhbrjPAxGRdfmAUUNElVzjcOdllA88KUOVRftFqR8m9fBGHEAQvVzIynBm3aWEpu5xeTTGd630wl+9FWotQCCJRlPP3fp7fmDJZmgnNqLbD4OFZJJbEQd6IeA215dTRCrS+Lwp+If/grXczBuog5QhNTg5wpvgIjESvWThqmrj7TJ6D4hJp3KUHZcFDzpujLjdOrzbFHCIfIK1TT82hML7GK7WGfVGYyZT7wYCnwKoZL84KYZxgbsg3x1xXDCN/Mj1ew/Oe3VmdZr272dUqDUqJsPkSWvNJ9t+1imW4qbf0VxQdwX5M5uVPBpN/bycwYbdL2LfSKlYBF4qdlJZ950Dg6qeKgK9KrodTPk+vLpUxguXcu5iK8RXxpKvmcgGp9UXGSxcUSGMyUw9SvF/WNruCJuh/UTj29mKAAAAAAbd9uHXZaGT2cvhRs7reawctIXtX1s3kTqM9YV+/wCphQ8tbgKkevgk0Jq2ncQtcMsoy/okn7fuV7nSVsEnYu/5BhhUacF5XhWELMujq2utWal/Fr1KPaTybEzGoKUSWwEMDAYEBwMBCAkAAgULCjMACgAAAAAAAACkQQEAAAAAAAEAAAAAAAAAEKR9AAAAAAAAAAAAAAAAAFzaUz9Wik8c//8=", 
        {
            "skipPreflight": true, 
            "preflightCommitment": "finalized", 
            "encoding": "base64"
        }
    ]
}

*/


std::string SerumMarket::send_transaction(Transaction &txn, const std::vector<SolKeyPair> &signers)
{

    auto blhs = get_latest_blockhash();
    txn.set_recent_blockhash(blhs);
    txn.sign(signers[0]);
    auto msg = txn.serialize();
    auto decode_msg = base64_encode(msg);
    string data_str;
    try{
        data_str = HttpClient::request(
            (boost::format(R"({
                "jsonrpc": "2.0", 
                "id": 5, 
                "method": "sendTransaction", 
                "params": [
                    "%1%", 
                    {
                        "skipPreflight": true, 
                        "preflightCommitment": "finalized", 
                        "encoding": "base64"
                    }
                ]
            })") % decode_msg).str(), 
            http_address_, 
            HttpClient::HTTPMethod::POST,
            std::vector<string>({"Content-Type: application/json"})
        );
    }
    catch(std::exception e) {

        return "";
    }
    return data_str;
}

void SerumMarket::new_order_v3(const NewOrderV3Params& params, SolInstruction& instruction) 
{
    instruction.program_id = params.program_id;
    instruction.accounts = new SolAccountMeta[12] {
        SolAccountMeta { pubkey: params.market, is_writable: true, is_signer: false },
        SolAccountMeta { pubkey: params.open_orders, is_writable: true, is_signer: false },
        SolAccountMeta { pubkey: params.request_queue, is_writable: true, is_signer: false },
        SolAccountMeta { pubkey: params.event_queue, is_writable: true, is_signer: false },
        SolAccountMeta { pubkey: params.bids, is_writable: true, is_signer: false },
        SolAccountMeta { pubkey: params.asks, is_writable: true, is_signer: false },
        SolAccountMeta { pubkey: params.payer, is_writable: true, is_signer: false },
        SolAccountMeta { pubkey: params.owner, is_writable: true, is_signer: true },
        SolAccountMeta { pubkey: params.base_vault, is_writable: true, is_signer: false },
        SolAccountMeta { pubkey: params.quote_vault, is_writable: true, is_signer: false },
        SolAccountMeta { pubkey: base58str_to_pubkey(TOKEN_PROGRAM_ID), is_writable: false, is_signer: false },
        SolAccountMeta { pubkey: base58str_to_pubkey(SYSVAR_RENT_PUBKEY), is_writable: false, is_signer: false }
    };
    instruction.account_len = 12;

    // auto order = NewOrderV3 {
    //         side: params.side,
    //         limit_price: params.limit_price,
    //         max_base_quantity: params.max_base_quantity,
    //         max_quote_quantity: params.max_quote_quantity,
    //         self_trade_behavior: params.self_trade_behavior,
    //         order_type: params.order_type,
    //         client_id: params.client_id,
    //         limit: 65535
    //     };

    auto ord_layout = InstructionLayoutOrderV3 {
        0,
        10,
        NewOrderV3 {
            side: params.side,
            limit_price: params.limit_price,
            max_base_quantity: params.max_base_quantity,
            max_quote_quantity: params.max_quote_quantity,
            self_trade_behavior: params.self_trade_behavior,
            order_type: params.order_type,
            client_id: params.client_id,
            limit: 65535
        }
    };
    serialize( 
        instruction.data,
        &ord_layout,  
        sizeof(InstructionLayoutOrderV3)
    );
    instruction.data_len = sizeof(InstructionLayoutOrderV3);
}

// void SerumMarket::deserialize(char* dest, void* src, size_t data_size)
// {
//     memcpy(str, data, data_size);
// }

// void SerumMarket::serialize(char* dest, const void* src, size_t data_size)
// {
//     memcpy(dest, src, data_size);
// }

uint64_t SerumMarket::price_number_to_lots(long double price, const MarketChannel& info)
{
    return static_cast<uint64_t>(
        (price * info.quote_spl_token_multiplier * info.parsed_market.base_lot_size) 
        / (info.base_spl_token_multiplier * info.parsed_market.quote_lot_size)
    ); 
}

uint64_t SerumMarket::base_size_number_to_lots(long double size, const MarketChannel& info)
{
    return static_cast<uint64_t>(
        std::floor(size * info.base_spl_token_multiplier) / info.parsed_market.base_lot_size
    );
}