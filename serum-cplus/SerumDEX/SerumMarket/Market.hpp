#pragma once 

#include "enums.hpp"
#include "models.hpp"
#include "constants.hpp"
#include "sysvar.hpp"
#include "MintAddresses.hpp"

#include <sol_sdk/enums.hpp>
#include <sol_sdk/PublicKey.hpp>
#include <sol_sdk/Keypair.hpp>
#include <sol_sdk/Transaction.hpp>
#include <base64/base64.h>
#include <SerumDEX/SerumTrade.h>
#include <boost/json.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <functional>
#include <string>
#include <map>
#include <ctime>
#include <cmath>
#include <random>

#include <marketlib/include/enums.h>
#include <marketlib/include/market.h>
#include <sharedlib/include/IPoolsRequester.h>
#include <sharedlib/include/ILogger.h>
#include <sharedlib/include/HTTPClient.h>
#include <sharedlib/include/IMarket.h>



#define LAMPORTS_PER_SOL 1000000000
#define MARKET_KEY PublicKey("srmqPvymJeFKQ4zGQed1GFppgkRHL9kaELCbyksJtPX")

class SerumMarket : IMarket
{
private:
    typedef std::string string;
    typedef std::shared_ptr < IPoolsRequester > pools_ptr;
    typedef std::shared_ptr < IListener > listener_ptr;
    // typedef std::shared_ptr < MintAdresses > MintAddresses_ptr;
    typedef marketlib::order_t Order;
    typedef Keypair SecretKey;
    typedef marketlib::execution_report_t ExecutionReport;
    typedef marketlib::instrument_descr_t Instrument;
    typedef std::function <void(const string&, const ExecutionReport&)> OrdersCallback;
    typedef std::shared_ptr< ILogger > logger_ptr;
    typedef solana::PublicKey PublicKey;

    struct MarketLayout
    {
        PublicKey request_queue;
        PublicKey event_queue;
        PublicKey bids;
        PublicKey asks;
        PublicKey base_vault;
        PublicKey quote_vault;
        uint64_t base_lot_size;
        uint64_t quote_lot_size;
    };

    struct MarketChannel
    {
        string symbol;
		Instrument instr;
        PublicKey market_address;
        // PublicKey open_order_account;
        MarketLayout parsed_market;
        uint64_t base_spl_token_multiplier;
        uint64_t quote_spl_token_multiplier;
        PublicKey payer_sell;
        PublicKey payer_buy;
    };

    using MarketChannels = boost::multi_index::multi_index_container<
        MarketChannel,
        boost::multi_index::indexed_by<
            boost::multi_index::hashed_unique<
                boost::multi_index::tag<struct MarketChannelsBySymbol>,
                boost::multi_index::member<MarketChannel, decltype(MarketChannel::symbol), &MarketChannel::symbol >
            >
        >
    >;

    using Orders = boost::multi_index::multi_index_container<
        Order ,
        boost::multi_index::indexed_by<
            boost::multi_index::hashed_unique<
                boost::multi_index::tag<struct OrderByCliId>,
                boost::multi_index::member<Order, decltype(Order::clId), &Order::clId >
            >
        >
    >;

    struct change_order_status
    {
        change_order_status(const marketlib::order_state_t& new_state):new_state_(new_state){}
        void operator()(Order& o)
        {
            o.state=new_state_;
        }

    private:
        marketlib::order_state_t new_state_;
    };

    PublicKey _pubkey;
    Keypair _secretkey;
    string _http_address;
    pools_ptr _pools;
    Orders _open_orders;
    OrdersCallback _orders_callback;
    listener_ptr _trade_channel;
    logger_ptr _logger;
    MarketChannels _markets_info;
    std::map<std::string, uint64_t> _order_count_for_symbol;
    // MintAddresses_ptr _mint_addresses;
    // pair - count
    std::map<string, uint64_t> _subscribed_channels;
    // std::map<string, string> _mint_addresses;
    uint64_t _message_count;
    const string _name;
    
    string place_order(
        const MarketChannel&,
        const OpenOrdersAccountInfo& orders_account_info,
        OrderType,
        Side,
        double,
        double,
        uint64_t,
        Transaction&,
        Transaction::Signers&,
        const PublicKey&
    );

    // void load_mint_addresses();
    MarketChannel create_market_info(const Instrument&, const PublicKey&);
    const MarketChannel& get_market_info(const Instrument&, const PublicKey&);
    MarketLayout get_market_layout(const string&);
    string get_account_info(const string&);
    string get_latest_blockhash();
    string get_minimum_balance_for_rent_exemption();
    string get_token_account_by_owner(const string&, const string&);
    string get_token_program_accounts(const string&, const string&, const string&);
    string send_transaction(Transaction& transaction, const Transaction::Signers&);
    uint8_t get_mint_decimals(const string&);
    OpenOrdersAccountInfo get_orders_account_info(const Instrument&, const PublicKey&);
    uint64_t get_balance_needed();

    Instruction new_order_v3(const NewOrderV3Params&) const;
    Instruction new_cancel_order_by_client_id_v2(const CancelOrderV2ByClientIdParams&) const;
    Instruction create_account(const CreateAccountParams&) const;
    Instruction initialize_account(const InitializeAccountParams&) const;
    Instruction close_account(const CloseAccountParams&) const;

    // precision
    uint64_t price_number_to_lots(long double price, const MarketChannel& info) const;
    uint64_t base_size_number_to_lots(long double price, const MarketChannel& info) const;
    uint64_t get_lamport_need_for_sol_wrapping(double, double, Side, const OpenOrdersAccountInfo&) const;

    time_t current_time() const { return std::time(nullptr);};

    // void order_checker(const string&, const string&, const ExecutionReport&);
    void check_order(const Instrument&);
    void uncheck_order(const Instrument&);

    std::string getMarketFromInstrument(const Instrument& instr) {
		if (instr.base_currency.size() && instr.quote_currency.size())
			return instr.base_currency + "/" + instr.quote_currency;
		return instr.symbol;
	}
public:
    SerumMarket(const string&, const string&, const string&, logger_ptr, pools_ptr, listener_ptr, OrdersCallback, const string& );
    // SerumMarket(const string&, pools_ptr, Callback, OrdersCallback);
    // SerumMarket(const SerumMarket&);

    Order send_new_order(const Instrument&, const Order&) override;
    Order cancel_order(const Instrument&, const string&) override;

    ~SerumMarket();
};