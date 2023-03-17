#include "Market.hpp"
using namespace solana; 

SerumMarket::SerumMarket(
    market_settings_ptr market_settings, logger_ptr logger,
    pools_ptr pools, listener_ptr listener, OrdersCallback orders_callback): 
_market_settings(market_settings), _logger(logger),
_pools(pools), _trade_channel(listener), _message_count(0), _orders_callback(orders_callback),
_order_count_for_symbol()
{}

SerumMarket::~SerumMarket()
{
    // _mint_addresses.clear();
    for (const auto& p : _order_count_for_symbol)
    {
        Instrument i;
        i.symbol = p.first;
        _trade_channel->unlisten(
            i, 
            get_name() + i.symbol
        );
    }
       
    _subscribed_channels.clear();
    _open_orders.clear();
}

SerumMarket::Order SerumMarket::send_new_order(const Instrument& instrument_, const Order& order_) 
{
    if (order_.type != marketlib::order_type_t::ot_Limit) {
        ExecutionReport execution_report;
        execution_report.clId        = order_.clId;
        execution_report.type        = marketlib::report_type_t::rt_rejected;
        execution_report.state       = marketlib::order_state_t::ost_Rejected;
        execution_report.text        = "OpenBook Market::OpenBook supports only limit orders";
            
        _orders_callback(get_name(), execution_report);
        return order_;
    }

    auto secretkey = get_private_key();
    auto pubkey = secretkey.get_pubkey();

    MarketChannel market_info;
    OpenOrdersAccountInfo orders_account_info;
    Transaction txn;
    Transaction::Signers signers;
    signers.push_back(secretkey);
    try {
        market_info = get_market_info(instrument_, pubkey);
        orders_account_info = get_orders_account_info(market_info.instr, pubkey);
        if (!orders_account_info.account.size())
            throw string("There is no open order account for this symbol " + market_info.instr.symbol);

        // if (orders_account_info.account.size() == 0) {
        //     auto balance_needed = get_balance_needed();
        //     auto new_open_order_account = Keypair();
        //     signers.push_back(new_open_order_account);
        //     txn.add_instruction(
        //         create_account(
        //             CreateAccountParams{
        //                 owner: _pubkey,
        //                 new_account: new_open_order_account.get_pubkey(),
        //                 lamports: balance_needed,
        //                 program_id: market_info.instr.address
        //             }
        //         )
        //     );
        //     orders_account_info.account = new_open_order_account.get_pubkey();
        // }

        if (order_.side == marketlib::order_side_t::os_Buy && market_info.payer_buy.get_str_key().empty()) {
            auto payer_buy = get_token_account_by_owner(pubkey.get_str_key(), market_info.instr.quote_mint_address);
            if (!payer_buy.empty()) {
                market_info.payer_buy = payer_buy;
            }
            else {
                auto balance_needed = get_balance_needed();
                Keypair payer_buy;
                market_info.payer_buy = payer_buy.get_pubkey();
                signers.push_back(payer_buy);
                txn.add_instruction(
                    create_account(
                        CreateAccountParams{
                            owner: pubkey,
                            new_account: payer_buy.get_pubkey(),
                            lamports: balance_needed,
                            program_id: PublicKey("11111111111111111111111111111111")
                        }
                    )
                );
            }
        }
    }
    catch (string e) {
        _logger->Error(( boost::format(R"(OpenBook Market::Failed to get information: %1%)") % e ).str().c_str());
        ExecutionReport execution_report;
        execution_report.clId       = order_.clId;
        //execution_report.orderType  = order_.type;
        execution_report.type       = marketlib::report_type_t::rt_rejected;
        //execution_report.transType  = marketlib::exec_trans_t::ett_undefined;
        execution_report.state      = marketlib::order_state_t::ost_Rejected;
        //execution_report.side       = order_.side;
        //execution_report.rejReason  = marketlib::ord_rej_reason::rr_other;
        execution_report.text       = ( boost::format(R"(OpenBook Market::Failed to get information: %1%)") % e ).str();

        _orders_callback(get_name(), execution_report);
        return order_;
    }

    auto order = order_;
    if (strtoul(order.clId.c_str(), nullptr, 0) == 0) {
        std::random_device rd; 
        std::mt19937_64 mersenne(rd());
        order.clId = std::to_string(mersenne());
    };
    // auto payer = order.side == marketlib::order_side_t::os_Buy ? market_info->payer_buy : market_info->payer_sell;

    try{
        auto res = place_order(
            market_info,
            orders_account_info,
            OrderType::LIMIT,
            order.side == marketlib::order_side_t::os_Buy ? Side::BUY : Side::SELL,
            order.price,
            order.original_qty,
            strtoul(order.clId.c_str(), nullptr, 0),
            txn,
            signers,
            pubkey
        );
        _logger->Debug(( boost::format(R"(OpenBook Market::The order is sent: %1%)") % res).str().c_str() );
        order.transaction_hash = string(boost::json::parse(res).at("result").as_string().c_str());
    }

    catch (string e) {
        _logger->Error(( boost::format(R"(OpenBook Market::Failed to send the order: %1%)") % e).str().c_str());
        ExecutionReport execution_report;
        execution_report.clId       = order_.clId;
        //execution_report.orderType  = order_.type;
        execution_report.type       = marketlib::report_type_t::rt_rejected;
        //execution_report.transType  = marketlib::exec_trans_t::ett_undefined;
        execution_report.state      = marketlib::order_state_t::ost_Rejected;
        //execution_report.side       = order_.side;
        //execution_report.rejReason  = marketlib::ord_rej_reason::rr_other;
        execution_report.text       = ( boost::format(R"(OpenBook Market::Failed to send the order: %1%)") % e).str();

        _orders_callback(get_name(), execution_report);
        return order_;
    }

    order.state = marketlib::order_state_t::ost_Undefined;
    order.init_time = current_time();
    _open_orders.insert(order);

    check_order(instrument_);
    return *(_open_orders.begin());
}

SerumMarket::Order SerumMarket::cancel_order(const Instrument& instrument, const string& client_id)
{

    auto secretkey = get_private_key();
    auto pubkey = secretkey.get_pubkey();

    MarketChannel market_info;
    OpenOrdersAccountInfo orders_account_info;
    try {
        market_info = get_market_info(instrument, pubkey);
        orders_account_info = get_orders_account_info(market_info.instr, pubkey);        
    }
    catch (string e) {
        _logger->Error(( boost::format(R"(OpenBook Market::Failed to get information: %1%)") % e ).str().c_str());
        ExecutionReport execution_report;
        execution_report.clId       = client_id;
        //execution_report.orderType  = marketlib::order_type_t::ot_Undefined;
        execution_report.type       = marketlib::report_type_t::rt_cancel_rejected;
        //execution_report.transType  = marketlib::exec_trans_t::ett_undefined;
        execution_report.state      = marketlib::order_state_t::ost_Rejected;
        //execution_report.side       = marketlib::order_side_t::os_Undefined;
        //execution_report.rejReason  = marketlib::ord_rej_reason::rr_other;
        execution_report.text       = ( boost::format(R"(OpenBook Market::Failed to get information: %1%)") % e ).str();

        _orders_callback(get_name(), execution_report);
        Order order;
        order.clId = client_id;
        return order;
    }

    CancelOrderV2ByClientIdParams cancelOrderParam;
    cancelOrderParam.market= market_info.market_address;
    cancelOrderParam.bids= market_info.parsed_market.bids;
    cancelOrderParam.asks= market_info.parsed_market.asks;
    cancelOrderParam.event_queue= market_info.parsed_market.event_queue;
    cancelOrderParam.open_orders= orders_account_info.account;
    cancelOrderParam.owner= pubkey;
    cancelOrderParam.client_id= strtoul(client_id.c_str(), nullptr, 0);
    cancelOrderParam.program_id= MARKET_KEY;

    Transaction txn;
    txn.add_instruction(new_cancel_order_by_client_id_v2(cancelOrderParam));

    /*
    Transaction txn;
    txn.add_instruction(
        new_cancel_order_by_client_id_v2(
            CancelOrderV2ByClientIdParams {
                market: market_info.market_address,
                bids: market_info.parsed_market.bids,
                asks: market_info.parsed_market.asks,
                event_queue: market_info.parsed_market.event_queue,
                open_orders: orders_account_info.account,
                owner: _pubkey,
                client_id: strtoul(client_id.c_str(), nullptr, 0),
                program_id: MARKET_KEY
            }
        )
    );
     */
    Transaction::Signers signers;
    signers.push_back(secretkey);


    Order order ;
    order.clId= client_id;    
    try{
        auto res = send_transaction(txn, signers);
        order.transaction_hash = string(boost::json::parse(res).at("result").as_string().c_str());
        _logger->Debug(( boost::format(R"(The order is sent: %1%)") % res).str().c_str());
    }
    catch (string e)
    {
        _logger->Error(( boost::format(R"(OpenBook Market::Failed to send the order: %1%)") % e).str().c_str());
        ExecutionReport execution_report;
        execution_report.clId        = client_id;
       //execution_report.orderType   = marketlib::order_type_t::ot_Undefined;
        execution_report.type        = marketlib::report_type_t::rt_cancel_rejected;
        //execution_report.transType   = marketlib::exec_trans_t::ett_undefined;
        execution_report.state       = marketlib::order_state_t::ost_Rejected;
        //execution_report.side        = marketlib::order_side_t::os_Undefined;
        //execution_report.rejReason   = marketlib::ord_rej_reason::rr_other;
        execution_report.text        = ( boost::format(R"(OpenBook Market::Failed to send the order: %1%)") % e).str();
        _orders_callback(get_name(), execution_report);
    }
    return order;
}

// // 'url': 'https://solana-api.projectserum.com', 
// // 'headers': {'Content-Type': 'application/json'}, 
// // 'data': '{"jsonrpc": "2.0", "id": 1, "method": "getAccountInfo", "params": ["9wFFyRfZBsuAha4YcuxcXLKwMxJR43S7fPfQLusDBzvT", {"encoding": "base64", "commitment": "finalized"}]}'}

SerumMarket::string SerumMarket::place_order(
    const MarketChannel& info_,
    const OpenOrdersAccountInfo& orders_account_info_,
    OrderType order_type_,
    Side side_,
    double limit_price_,
    double max_quantity_,
    uint64_t client_id_,
    Transaction& txn_,
    Transaction::Signers& signers_,
    const PublicKey& owner_pubkey_
)
{
    bool should_wrap_sol = (side_ == Side::BUY && info_.instr.quote_mint_address == WRAPPED_SOL_MINT) || 
    (side_ == Side::SELL && info_.instr.base_mint_address == WRAPPED_SOL_MINT);

    Keypair wrapped_sol_account;
    PublicKey payer = side_ == Side::BUY ? info_.payer_buy : info_.payer_sell;
    if (should_wrap_sol)
    {
        payer = wrapped_sol_account.get_pubkey();
        signers_.push_back(wrapped_sol_account);
        txn_.add_instruction(
            create_account(
                CreateAccountParams{
                    owner: owner_pubkey_,
                    new_account: payer,
                    lamports: get_lamport_need_for_sol_wrapping(
                        limit_price_, max_quantity_, side_, orders_account_info_
                    ),
                    program_id: PublicKey("11111111111111111111111111111111")
                }
            )
        );
        txn_.add_instruction(
            initialize_account(
                InitializeAccountParams{
                    account: wrapped_sol_account.get_pubkey(),
                    mint: WRAPPED_SOL_MINT,
                    owner: owner_pubkey_,
                    program_id: TOKEN_PROGRAM_ID
                }
            )
        );
    }
    
    txn_.add_instruction(
        new_order_v3(
            NewOrderV3Params{
                market: info_.market_address,
                open_orders: orders_account_info_.account,
                payer: payer,
                owner: owner_pubkey_,
                request_queue: info_.parsed_market.request_queue,
                event_queue: info_.parsed_market.event_queue,
                bids: info_.parsed_market.bids,
                asks: info_.parsed_market.asks,
                base_vault: info_.parsed_market.base_vault,
                quote_vault: info_.parsed_market.quote_vault,
                side: side_,
                limit_price: price_number_to_lots(limit_price_, info_),
                max_base_quantity: base_size_number_to_lots(max_quantity_, info_), 
                max_quote_quantity: base_size_number_to_lots(max_quantity_, info_) 
                * info_.parsed_market.quote_lot_size
                * price_number_to_lots(limit_price_, info_),
                order_type: order_type_,
                self_trade_behavior: SelfTradeBehavior::DECREMENT_TAKE,
                limit: 65535,
                client_id: client_id_,
                program_id: MARKET_KEY
            }
        )
    );
    
    // signers.push_back(secretkey_);
    if(should_wrap_sol) {
        txn_.add_instruction(
            close_account(
                CloseAccountParams {
                    account: wrapped_sol_account.get_pubkey(),
                    owner: owner_pubkey_,
                    dest: owner_pubkey_,
                    program_id: TOKEN_PROGRAM_ID
                }
            )
        );
    }
    
    return send_transaction(txn_, signers_);
}

const SerumMarket::MarketChannel& SerumMarket::get_market_info(const Instrument& instrument_, const PublicKey& pubkey_)
{
    auto market_info = _markets_info.get<MarketChannelsBySymbol>()
		.find(instrument_.symbol);

    if (market_info == _markets_info.end()) {
        auto pool = _pools->getPool(instrument_);
        _markets_info.insert(create_market_info(pool, pubkey_));
        market_info = _markets_info.begin();
    }

    return *market_info;
}

// TODO
SerumMarket::MarketChannel SerumMarket::create_market_info(const Instrument& instr_, const PublicKey& pubkey_)
{
    return MarketChannel {
        symbol: instr_.symbol,
        instr: instr_,
        market_address: instr_.address,
        parsed_market: get_market_layout(instr_.address),
        base_spl_token_multiplier: static_cast<uint64_t>(pow(10, get_mint_decimals(instr_.base_mint_address))),
        quote_spl_token_multiplier: static_cast<uint64_t>(pow(10, get_mint_decimals(instr_.quote_mint_address))),
        payer_sell: pubkey_
    };
}

SerumMarket::MarketLayout SerumMarket::get_market_layout(const string& market_address_)
{
    string account_data = base64_decode(
        string(boost::json::parse(get_account_info(market_address_)).at("result").at("value").at("data").as_array()[0].as_string().c_str())
    );

    SolMarketLayout market; 
    memcpy((void*)&market, account_data.data(), account_data.size());

    return MarketLayout {
        request_queue: PublicKey(market.request_queue),
        event_queue: PublicKey(market.event_queue),
        bids: PublicKey(market.bids),
        asks: PublicKey(market.asks),
        base_vault: PublicKey(market.base_vault),
        quote_vault: PublicKey(market.quote_vault),
        base_lot_size: market.base_lot_size,
        quote_lot_size: market.quote_lot_size
    };
}

uint64_t SerumMarket::get_balance_needed()
{
    return boost::json::parse(get_minimum_balance_for_rent_exemption())
    .at("result")
    .as_int64();;
}

OpenOrdersAccountInfo SerumMarket::get_orders_account_info(const Instrument& instr_, const PublicKey& pubkey_)
{
    auto orders_accounts_info = get_token_program_accounts(
        MARKET_KEY.get_str_key(),
        instr_.address,
        pubkey_.get_str_key()
    );

    if (orders_accounts_info.empty() || boost::json::parse(orders_accounts_info).at("result").as_array().empty())
        return OpenOrdersAccountInfo{
            account: PublicKey(),
            base_token_free: 0,
            quote_token_free: 0
        };

    auto decoded = base64_decode(string(boost::json::parse(orders_accounts_info)
        .at("result")
        .as_array()[0]
        .at("account")
        .at("data")
        .as_array()[0].as_string().c_str()));

    auto open_order_layout = SolOpenOrderLayout{};
    memcpy(&open_order_layout, decoded.data(), sizeof(SolOpenOrderLayout));
    //boost::json::parse(data_str).at("result").as_array()[0].at("pubkey").as_string().c_str();

    return OpenOrdersAccountInfo {
        account: PublicKey(boost::json::parse(orders_accounts_info).at("result").as_array()[0].at("pubkey").as_string().c_str()),
        base_token_free: open_order_layout.base_token_free,
        quote_token_free: open_order_layout.quote_token_free
    };
}

uint8_t SerumMarket::get_mint_decimals(const string& mint_address_)
{
    if (mint_address_ == WRAPPED_SOL_MINT) 
        return 9;

    return (uint8_t)boost::json::parse(get_account_info(mint_address_))
        .at("result")
        .at("value")
        .at("data")
        .at("parsed")
        .at("info")
        .at("decimals").as_int64();
}

Instruction SerumMarket::new_cancel_order_by_client_id_v2(const CancelOrderV2ByClientIdParams& params_) const
{
    Instruction instruction;
    instruction.set_account_id(params_.program_id);
    instruction.set_accounts( Instruction::AccountMetas({
        Instruction::AccountMeta { pubkey: params_.market, is_writable: false, is_signer: false },
        Instruction::AccountMeta { pubkey: params_.bids, is_writable: true, is_signer: false },
        Instruction::AccountMeta { pubkey: params_.asks, is_writable: true, is_signer: false },
        Instruction::AccountMeta { pubkey: params_.open_orders, is_writable: true, is_signer: false },
        Instruction::AccountMeta { params_.owner, is_writable: true, is_signer: true },
        Instruction::AccountMeta { params_.event_queue, is_writable: true, is_signer: false }
    }));
    auto ord_layout = InstructionLayoutCancelOrderByClientIdV2 {
        0,
        ::InstructionType::CANCEL_ORDER_BY_CLIENT_ID_V2,
        CancelOrderByClientIdV2{
            order_id: params_.client_id
        }
    };
    instruction.set_data(&ord_layout, sizeof(InstructionLayoutCancelOrderByClientIdV2));
    return instruction;
}

Instruction SerumMarket::new_order_v3(const NewOrderV3Params& params_) const
{
    Instruction instruction;
    instruction.set_account_id(params_.program_id);
    instruction.set_accounts( Instruction::AccountMetas({
        Instruction::AccountMeta { pubkey: params_.market, is_writable: true, is_signer: false },
        Instruction::AccountMeta { pubkey: params_.open_orders, is_writable: true, is_signer: false },
        Instruction::AccountMeta { pubkey: params_.request_queue, is_writable: true, is_signer: false },
        Instruction::AccountMeta { pubkey: params_.event_queue, is_writable: true, is_signer: false },
        Instruction::AccountMeta { pubkey: params_.bids, is_writable: true, is_signer: false },
        Instruction::AccountMeta { pubkey: params_.asks, is_writable: true, is_signer: false },
        Instruction::AccountMeta { pubkey: params_.payer, is_writable: true, is_signer: false },
        Instruction::AccountMeta { pubkey: params_.owner, is_writable: true, is_signer: true },
        Instruction::AccountMeta { pubkey: params_.base_vault, is_writable: true, is_signer: false },
        Instruction::AccountMeta { pubkey: params_.quote_vault, is_writable: true, is_signer: false },
        Instruction::AccountMeta { pubkey: TOKEN_PROGRAM_ID, is_writable: false, is_signer: false },
        Instruction::AccountMeta { pubkey: SYSVAR_RENT_PUBKEY, is_writable: false, is_signer: false }
    }));

    auto ord_layout = InstructionLayoutOrderV3 {
        0,
        10,
        NewOrderV3 {
            side: params_.side,
            limit_price: params_.limit_price,
            max_base_quantity: params_.max_base_quantity,
            max_quote_quantity: params_.max_quote_quantity,
            self_trade_behavior: params_.self_trade_behavior,
            order_type: params_.order_type,
            client_id: params_.client_id,
            limit: 65535
        }
    };
    instruction.set_data(&ord_layout, sizeof(InstructionLayoutOrderV3));
    return instruction;
}

Instruction SerumMarket::create_account(const CreateAccountParams& params_) const
{
    Instruction instruction;
    instruction.set_account_id(params_.program_id);
    instruction.set_accounts( Instruction::AccountMetas({
        Instruction::AccountMeta { pubkey: params_.owner, is_writable: true, is_signer: true },
        Instruction::AccountMeta { pubkey: params_.new_account, is_writable: true, is_signer: true }
    }));

    auto ord_layout = InstructionLayoutCreateOrder {
        0,
        params_.lamports,
        ACCOUNT_LEN
    };
    memcpy(ord_layout.owner, TOKEN_PROGRAM_ID.data(), SIZE_PUBKEY);
    instruction.set_data(&ord_layout, sizeof(InstructionLayoutCreateOrder));
    return instruction;
}

// Instruction SerumMarket::create_open_orders_account(const CreateOOAccountParams& params_) const
// {
//     Instruction instruction;
//     instruction.set_account_id(params_.program_id);
//     instruction.set_accounts( Instruction::AccountMetas({
//         Instruction::AccountMeta { pubkey: params_.owner, is_writable: true, is_signer: true },
//         Instruction::AccountMeta { pubkey: params_.new_account, is_writable: true, is_signer: true }
//     }));

//     auto ord_layout = InstructionLayoutCreateOrder {
//         0,
//         params_.lamports,
//         ACCOUNT_LEN
//     };
//     memcpy(ord_layout.owner, TOKEN_PROGRAM_ID.data(), SIZE_PUBKEY);
//     instruction.set_data(&ord_layout, sizeof(InstructionLayoutCreateOrder));
//     return instruction;
// }

Instruction SerumMarket::initialize_account(const InitializeAccountParams& params_) const
{
    Instruction instruction;
    instruction.set_account_id(params_.program_id);
    instruction.set_accounts( Instruction::AccountMetas({
        Instruction::AccountMeta { pubkey: params_.account, is_writable: true, is_signer: true },
        Instruction::AccountMeta { pubkey: params_.mint, is_writable: false, is_signer: false },
        Instruction::AccountMeta { pubkey: params_.owner, is_writable: true, is_signer: true },
        Instruction::AccountMeta { pubkey: SYSVAR_RENT_PUBKEY, is_writable: false, is_signer: false }
    }));
    auto data = Instruction::bytes();
    data.push_back((uint8_t)solana::InstructionType::INITIALIZE_ACCOUNT);
    instruction.set_data(data);
    return instruction;
}

Instruction SerumMarket::close_account(const CloseAccountParams& params_) const
{
    Instruction instruction;
    instruction.set_account_id(params_.program_id);
    instruction.set_accounts( Instruction::AccountMetas({
        Instruction::AccountMeta { pubkey: params_.account, is_writable: true, is_signer: true },
        Instruction::AccountMeta { pubkey: params_.owner, is_writable: true, is_signer: true },
        Instruction::AccountMeta { pubkey: params_.dest, is_writable: true, is_signer: true },
    }));
    auto data = Instruction::bytes();
    data.push_back((uint8_t)solana::InstructionType::CLOSE_ACCOUNT);
    instruction.set_data(data);
    return instruction;
}

// void SerumMarket::get_mint_addresses()
// {
//     string data_str;
//     string address = "https://raw.githubusercontent.com/project-serum/serum-ts/master/packages/serum/src/token-mints.json";
//     try{
//         data_str = HttpClient::request(
//             "", 
//             address, 
//             HttpClient::HTTPMethod::GET
//         );
//     }
//     catch(std::exception e) {
//         throw string("Failed to make a request to " + address);
//     }

//     auto data = boost::json::parse(data_str).as_array();
//     for(const auto& el : data) {
//         mint_addresses_[el.at("name").as_string().c_str()] = el.at("address").as_string().c_str();
//     }
// }

// void SerumMarket::load_mint_addresses()
// {
//     string data_str;
//     string address = "https://raw.githubusercontent.com/solana-labs/token-list/main/src/tokens/solana.tokenlist.json";
//     try{
//         data_str = HttpClient::request(
//             "", 
//             address, 
//             HttpClient::HTTPMethod::GET
//         );
//     }
//     catch(std::exception e) {
//         throw string("Failed to make a request to " + address);
//     }

//     auto data = boost::json::parse(data_str).at("tokens").as_array();
//     for(const auto& el : data) {
//         _mint_addresses[el.at("symbol").as_string().c_str()] = el.at("address").as_string().c_str();
//     }
// }

std::string SerumMarket::get_latest_blockhash()
{
    string data_str;
    try{
        data_str = HttpClient::request(
            (boost::format(R"({
                "jsonrpc": "2.0", 
                "id": "%1%", 
                "method": "getLatestBlockhash", 
                "params": [{"commitment": "finalized"}]
            })") % ++_message_count).str(), 
            get_endpoint(), 
            HttpClient::HTTPMethod::POST,
            std::vector<string>({"Content-Type: application/json"})
        );
    }
    catch(string e) {
        throw string("Failed to make a request to " + get_endpoint()  + " error: " + e);
    }
    if (data_str.find("error") != std::string::npos) {
        throw string("Bad request to " + get_endpoint()  + " error: " + data_str);
    }
    return boost::json::parse(data_str).at("result").at("value").at("blockhash").as_string().c_str();
}

std::string SerumMarket::get_token_account_by_owner(const string& owner_pubkey_, const string& token_address_) 
{
    string data_str;
    try{
        data_str = HttpClient::request(
            (boost::format(R"({
                "jsonrpc": "2.0", 
                "id": "%1%", 
                "method": "getTokenAccountsByOwner", 
                "params": [
                    "%2%", 
                    {"mint": "%3%"}, 
                    {"commitment": "finalized", "encoding": "base64"}
                ]
            })") % ++_message_count % owner_pubkey_ % token_address_).str(), 
            get_endpoint(), 
            HttpClient::HTTPMethod::POST,
            std::vector<string>({"Content-Type: application/json"})
        );
    }
    catch(string e) {
        throw string("Failed to make a request to " + get_endpoint()  + " error: " + e);
    }
    if (data_str.find("error") != std::string::npos) {
        throw string("Bad request to " + get_endpoint()  + " error: " + data_str);
    }
    if (boost::json::parse(data_str).at("result").at("value").as_array().empty()) {
        return "";
    }

    try {
        return boost::json::parse(data_str).at("result").at("value").as_array()[0].at("pubkey").as_string().c_str();
    }
    catch (std::exception e){
        throw string("Failed to retrieve the data");
    }
}

SerumMarket::string SerumMarket::get_minimum_balance_for_rent_exemption()
{
    string data_str;
    try{
        data_str = HttpClient::request(
            (boost::format(R"({
                "jsonrpc": "2.0", 
                "id": "%1%", 
                "method": 
                "getMinimumBalanceForRentExemption", 
                "params": [%2%, {"commitment": "finalized"}]
            })") % ++_message_count % sizeof(SolOpenOrderLayout)).str(), 
            get_endpoint(), 
            HttpClient::HTTPMethod::POST,
            std::vector<string>({"Content-Type: application/json"})
        );
    }
    catch(string e) {
        throw string("Failed to make a request to " + get_endpoint()  + " error: " + e);
    }
    if (data_str.find("error") != std::string::npos) {
        throw string("Bad request to " + get_endpoint()  + " error: " + data_str);
    }
    return data_str;
}

// '{"jsonrpc": "2.0", "id": 3, "method": "getProgramAccounts", "params": ["9xQeWvG816bUx9EPjHmaT23yvVM2ZWbrrpZb9PusVFin", {"filters": [{"memcmp": {"offset": 13, "bytes": "9wFFyRfZBsuAha4YcuxcXLKwMxJR43S7fPfQLusDBzvT"}}, {"memcmp": {"offset": 45, "bytes": "GKvwL3FmQRHuB9mcZ3WuqTuVjbGDzdW51ec8fYdeHae1"}}, {"dataSize": 3228}], "encoding": "base64", "commitment": "recent"}]}'
SerumMarket::string SerumMarket::get_token_program_accounts(const string& market_key_, const string& pool_key_, const string& pubkey_owner_)
{
    string data_str;
    try{
        data_str = HttpClient::request(
            (boost::format(R"({
                "jsonrpc": "2.0", 
                "id": "%1%", 
                "method": "getProgramAccounts", 
                "params": [
                    "%2%", 
                    {"filters": [
                        {"memcmp": {"offset": 13, "bytes": "%3%"}}, 
                        {"memcmp": {"offset": 45, "bytes": "%4%"}}, 
                        {"dataSize": 3228}
                    ], 
                    "encoding": "base64", 
                    "commitment": "recent"}
                ]
            })") % ++_message_count % market_key_ % pool_key_ % pubkey_owner_).str(), 
            get_endpoint(), 
            HttpClient::HTTPMethod::POST,
            std::vector<string>({"Content-Type: application/json"})
        );
    }
    catch(string e) {
        throw string("Failed to make a request to " + get_endpoint()  + " error: " + e);
    }
    if (data_str.find("error") != std::string::npos) {
        throw string("Bad request to " + get_endpoint()  + " error: " + data_str);
    }
    return data_str ;
}

//'{"jsonrpc": "2.0", "id": 1, "method": "getAccountInfo", "params": ["9wFFyRfZBsuAha4YcuxcXLKwMxJR43S7fPfQLusDBzvT", {"encoding": "base64", "commitment": "finalized"}]}'
std::string SerumMarket::get_account_info(const string& account_)
{
    string data_str;
    try{
        data_str = HttpClient::request(
            (boost::format(R"({
                "jsonrpc": "2.0", 
                "id": "%1%", 
                "method": "getAccountInfo", 
                "params": [
                    "%2%", 
                    {"encoding": "jsonParsed", "commitment": "finalized"}
                ]
            })") % ++_message_count % account_).str(), 
            get_endpoint(), 
            HttpClient::HTTPMethod::POST,
            std::vector<string>({"Content-Type: application/json"})
        );
    }
    catch(string e) {
        throw string("Failed to make a request to " + get_endpoint()  + " error: " + e);
    }
    if (data_str.find("error") != std::string::npos) {
        throw string("Bad request to " + get_endpoint()  + " error: " + data_str);
    }
    return data_str;
}

std::string SerumMarket::send_transaction(Transaction &txn_, const Transaction::Signers &signers_)
{
    txn_.set_recent_blockhash(get_latest_blockhash());
    txn_.sign(signers_);
    auto msg = txn_.serialize();

    // auto hex_msg = to_hex_string(msg);
    // std::cout << hex_msg << std::endl;

    auto decode_msg = base64_encode(msg);
    string data_str;
    try{
        data_str = HttpClient::request(
            (boost::format(R"({
                "jsonrpc": "2.0", 
                "id": "%1%", 
                "method": "sendTransaction", 
                "params": [
                    "%2%", 
                    {
                        "skipPreflight": false, 
                        "preflightCommitment": "finalized", 
                        "encoding": "base64"
                    }
                ]
            })") % ++_message_count % decode_msg).str(), 
            get_endpoint(), 
            HttpClient::HTTPMethod::POST,
            std::vector<string>({"Content-Type: application/json"})
        );
    }
    catch(string e) {
        throw string("Failed to make a request to " + get_endpoint()  + " error: " + e);
    }
    if (data_str.find("error") != std::string::npos) {
        throw string("Bad request to " + get_endpoint()  + " error: " + data_str);
    }
    return data_str;
}

// void SerumMarket::order_checker(const string& exch_name_, const string& cli_id_, const ExecutionReport& exec_report_)  
// {
//     auto order = _open_orders.get<OrderByCliId>()
//     .find(exec_report_.clId);

//     if (order == _open_orders.end()) 
//         return;

//     _open_orders.modify(order, change_order_status(exec_report_.state));
//     _orders_callback(get_name(), exec_report_);

//     if (order->isCompleted()) {
//         _open_orders.erase(order);
//         uncheck_order(Instrument{});
//     }
// }

void SerumMarket::check_order(const Instrument& instrument_) 
{
    auto symbol = getMarketFromInstrument(instrument_);
    if (_order_count_for_symbol.find(symbol) != _order_count_for_symbol.end()) {
        ++_order_count_for_symbol[symbol];
        return;
    }

    auto checker = [this, instrument_]
        (const string& exch_name_, const string& cli_id_, const ExecutionReport& exec_report_)  
        {
            if (!_open_orders.size())
                return;
            auto order = _open_orders.get<OrderByCliId>()
            .find(exec_report_.clId);

            if (order == _open_orders.end())
                return;

            _open_orders.modify(order, change_order_status(exec_report_.state));

            ExecutionReport report;
            report.tradeId=            order->transaction_hash;
            report.clId=               exec_report_.clId;
            report.origClId=           exec_report_.origClId;
            report.exchId=             exec_report_.exchId;
            report.secId=              exec_report_.secId;
            report.transaction_hash=   order->transaction_hash;
            report.time=               exec_report_.time;
            report.orderType=          exec_report_.orderType;
            report.type=               exec_report_.type;
            report.transType=          exec_report_.transType;
            report.tif=                exec_report_.tif;
            report.state=              exec_report_.state;
            report.side=               exec_report_.side;
            report.rejReason=          exec_report_.rejReason;
            report.limitPrice=         exec_report_.limitPrice;
            report.avgPx=              exec_report_.avgPx;
            report.lastPx=             exec_report_.lastPx;
            report.leavesQty=          exec_report_.leavesQty;
            report.cumQty=             exec_report_.cumQty;
            report.lastShares=         exec_report_.lastShares;
            report.text=               exec_report_.text;
            _orders_callback(get_name(), report);

            if (order->isCompleted()) {
                _open_orders.erase(order);
                // uncheck_order(instrument_);
            }
        };

    _trade_channel->listen(
        instrument_, 
        get_name() + symbol, 
       checker
    );
    _order_count_for_symbol[symbol] = 1;
}

void SerumMarket::uncheck_order(const Instrument& instrument_)
{
    auto symbol = getMarketFromInstrument(instrument_);
    if (_order_count_for_symbol.find(symbol) == _order_count_for_symbol.end()) {
        return; 
    }
    
    --_order_count_for_symbol[symbol];
    if (_order_count_for_symbol[symbol] < 1) {
        _trade_channel->unlisten(
            instrument_, 
            get_name() + symbol
        );
        _order_count_for_symbol.erase(symbol);
    }
}

SerumMarket::string SerumMarket::get_name()
{
    return _market_settings->get(IMarketSettings::Property::ClientId);
}

SerumMarket::string SerumMarket::get_endpoint()
{
    return _market_settings->get(IMarketSettings::Property::Endpoint);
}

Keypair SerumMarket::get_private_key()
{
    return Keypair(_market_settings->get(IMarketSettings::Property::PrivateKey));
}


uint64_t SerumMarket::price_number_to_lots(long double price_, const MarketChannel& info_) const
{
    return static_cast<uint64_t>(
        (price_ * info_.quote_spl_token_multiplier * info_.parsed_market.base_lot_size) 
        / (info_.base_spl_token_multiplier * info_.parsed_market.quote_lot_size)
    ); 
}

uint64_t SerumMarket::base_size_number_to_lots(long double size_, const MarketChannel& info_) const
{
    return static_cast<uint64_t>(
        std::floor(size_ * info_.base_spl_token_multiplier) / info_.parsed_market.base_lot_size
    );
}

uint64_t SerumMarket::get_lamport_need_for_sol_wrapping(double limit_price_, double max_quantity_, Side side_, const OpenOrdersAccountInfo& orders_account_info_) const
{
    uint64_t lamports = 0;

    if (side_ == Side::BUY) {
        lamports = static_cast<uint64_t>(limit_price_ * max_quantity_ * 1.01 * LAMPORTS_PER_SOL);
        lamports -= orders_account_info_.quote_token_free;
    }
    else {
        lamports = static_cast<uint64_t>(max_quantity_ * LAMPORTS_PER_SOL);
        lamports -= orders_account_info_.base_token_free;
    }

    return std::max(lamports, (uint64_t)0) + 10000000;
}