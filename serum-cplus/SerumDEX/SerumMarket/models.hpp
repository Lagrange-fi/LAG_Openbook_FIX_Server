#pragma once
#include "enums.hpp"
#include <sol_sdk/PublicKey.hpp>
#include <string>
#include <stdint.h>

using namespace solana;

#define VERSION 0

struct Mintlayout
{
    uint8_t padding_1[44];
    uint8_t decimals;
    uint8_t padding_2[37];
};

struct NewOrderV3Params 
{
    PublicKey market;
    PublicKey open_orders;
    PublicKey payer;
    PublicKey owner;
    PublicKey request_queue;
    PublicKey event_queue;
    PublicKey bids;
    PublicKey asks;
    PublicKey base_vault;
    PublicKey quote_vault;
    Side side;
    uint64_t limit_price;
    uint64_t max_base_quantity;
    uint64_t max_quote_quantity;
    OrderType order_type;
    SelfTradeBehavior self_trade_behavior;
    uint64_t limit;
    uint64_t client_id;
    PublicKey program_id;
};

struct CreateAccountParams
{
    PublicKey owner;
    PublicKey new_account;
    uint64_t lamports;
    uint64_t space;
    PublicKey program_id;
};

struct InitializeMarket
{
    uint64_t base_lot_size;
    uint64_t quote_lot_size;
    uint16_t fee_rate_bps;
    uint64_t vault_signer_nonce;
    uint64_t quote_dust_threshold;
};

struct CancelOrderV2ByClientIdParams
{
    PublicKey market;
    PublicKey bids;
    PublicKey asks;
    PublicKey event_queue;
    PublicKey open_orders;
    PublicKey owner;
    uint64_t client_id;
    PublicKey program_id;
};

struct CancelOrderV2Params
{
    PublicKey market;
    PublicKey bids;
    PublicKey asks;
    PublicKey event_queue;
    PublicKey open_orders;
    PublicKey owner;
    Side side;
    uint8_t order_id[16];
    // uint64_t open_orders_slot;
    PublicKey program_id;
};

struct InitializeAccountParams
{
    PublicKey account;
    PublicKey mint;
    PublicKey owner;
    // PublicKey sysvar;
    PublicKey program_id;
};

struct CloseAccountParams
{
    PublicKey account;
    PublicKey owner;
    PublicKey dest;
    PublicKey program_id;
};

struct OpenOrdersAccountInfo
{
    PublicKey account;
    uint64_t base_token_free;
    uint64_t quote_token_free;
};

#pragma pack(push,1)
struct NewOrderV3
{
    uint32_t side;
    uint64_t limit_price;
    uint64_t max_base_quantity;
    uint64_t max_quote_quantity;
    uint32_t self_trade_behavior;
    uint32_t order_type;
    uint64_t client_id;
    uint16_t limit;
};

struct InstructionLayoutOrderV3
{
    uint8_t version {0};
    uint32_t type {10};
    NewOrderV3 order;
};

struct CancelOrderV2
{
    uint32_t side;
    uint8_t order_id[16];
};

struct CancelOrderByClientIdV2
{
    uint64_t order_id;
};

struct InstructionLayoutCancelOrderV2
{
    uint8_t version {0};
    uint32_t type {11};
    CancelOrderV2 order;
};

struct InstructionLayoutCancelOrderByClientIdV2
{
    uint8_t version {0};
    uint32_t type {12};
    CancelOrderByClientIdV2 order;
};

struct InstructionLayoutCreateOrder
{
    uint32_t type {0};
    uint64_t lamports;
    uint64_t space;
    uint8_t owner[SIZE_PUBKEY];
};

struct SolMarketLayout
{
    uint8_t serum[5];
    uint64_t account_flags;
    uint8_t own_address[SIZE_PUBKEY];
    uint64_t vault_signer_nonce;
    uint8_t base_mint[SIZE_PUBKEY];
    uint8_t quote_mint[SIZE_PUBKEY];
    uint8_t base_vault[SIZE_PUBKEY];
    uint64_t base_deposits_total;
    uint64_t base_fees_accrued;
    uint8_t quote_vault[SIZE_PUBKEY];
    uint64_t quote_deposits_total;
    uint64_t quote_fees_accrued;
    uint64_t quote_dust_threshold;
    uint8_t request_queue[SIZE_PUBKEY];
    uint8_t event_queue[SIZE_PUBKEY];
    uint8_t bids[SIZE_PUBKEY];
    uint8_t asks[SIZE_PUBKEY];
    uint64_t base_lot_size;
    uint64_t quote_lot_size;
    uint64_t fee_rate_bps;
    uint64_t referrer_rebate_accrued;
    uint8_t padding[7];
};

struct SolOpenOrderLayout
{
    uint8_t serum[5];
    uint64_t account_flags;
    uint8_t market[SIZE_PUBKEY];
    uint8_t owner[SIZE_PUBKEY];
    uint64_t base_token_free;
    uint64_t base_token_total;
    uint64_t quote_token_free;
    uint64_t quote_token_total;
    __uint128_t free_slot_bits;
    __uint128_t is_bid_bits;
    __uint128_t orders[128];
    uint64_t client_ids[128];
    uint64_t referrer_rebate_accrued;
    uint8_t padding[7];
};
#pragma pack(pop)


