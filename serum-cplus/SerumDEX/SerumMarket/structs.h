#pragma once
#include "sol_sdk/types.h"
#include "sol_sdk/key.h"
#include "enums.h"
#include <string>
#define VERSION 0


struct MarketLayout
{
    // uint8_t serum[5];
    uint64_t account_flags;
    SolPubkey own_address;
    uint64_t vault_signer_nonce;
    SolPubkey base_mint;
    SolPubkey quote_mint;
    SolPubkey base_vault;
    uint64_t base_deposits_total;
    uint64_t base_fees_accrued;
    SolPubkey quote_vault;
    uint64_t quote_deposits_total;
    uint64_t quote_fees_accrued;
    uint64_t quote_dust_threshold;
    SolPubkey request_queue;
    SolPubkey event_queue;
    SolPubkey bids;
    SolPubkey asks;
    uint64_t base_lot_size;
    uint64_t quote_lot_size;
    uint64_t fee_rate_bps;
    uint64_t referrer_rebate_accrued;
    // uint8_t padding[7];
};

// struct MarketLayout
// {
//     // uint8_t serum[5];
//     uint64_t account_flags;
//     uint8_t own_address[32];
//     uint64_t vault_signer_nonce;
//     uint8_t base_mint[32];
//     uint8_t quote_mint[32];
//     uint8_t base_vault[32];
//     uint64_t base_deposits_total;
//     uint64_t base_fees_accrued;
//     uint8_t quote_vault[32];
//     uint64_t quote_deposits_total;
//     uint64_t quote_fees_accrued;
//     uint64_t quote_dust_threshold;
//     uint8_t request_queue[32];
//     uint8_t event_queue[32];
//     uint8_t bids[32];
//     uint8_t asks[32];
//     uint64_t base_lot_size;
//     uint64_t quote_lot_size;
//     uint64_t fee_rate_bps;
//     uint64_t referrer_rebate_accrued;
//     // uint8_t padding[7];
// };

struct Mintlayout
{
    uint8_t padding_1[44];
    uint8_t decimals;
    uint8_t padding_2[37];
};

struct NewOrderV3Params 
{
    SolPubkey market;
    SolPubkey open_orders;
    SolPubkey payer;
    SolPubkey owner;
    SolPubkey request_queue;
    SolPubkey event_queue;
    SolPubkey bids;
    SolPubkey asks;
    SolPubkey base_vault;
    SolPubkey quote_vault;
    Side side;
    uint64_t limit_price;
    uint64_t max_base_quantity;
    uint64_t max_quote_quantity;
    OrderType order_type;
    SelfTradeBehavior self_trade_behavior;
    uint64_t limit;
    uint64_t client_id;
    SolPubkey program_id;//= base58str_to_pubkey("9xQeWvG816bUx9EPjHmaT23yvVM2ZWbrrpZb9PusVFin");
};

struct Order
{
  std::string first;
  std::string second;
  double amount;
  double price;
  Side side;
  long long unsigned int client_id = 0;
};

struct InitializeMarket
{
    uint64_t base_lot_size;
    uint64_t quote_lot_size;
    uint16_t fee_rate_bps;
    uint64_t vault_signer_nonce;
    uint64_t quote_dust_threshold;
};

// struct NewOrder
// {
//     uint32_t side;
//     uint64_t limit_price;
//     uint64_t max_quantity;
//     uint32_t order_type;
//     uint64_t client_id;
// };



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

struct InstructionLayoutCancelOrderV2
{
    uint8_t version {0};
    uint32_t type {11};
    CancelOrderV2 order;
};
#pragma pack(pop)

// struct CancelOrder
// {
//     uint32_t side;
//     uint8_t order_id[16];
//     uint8_t open_orders[32];
//     uint8_t open_orders_slot;
// };

struct CancelOrderV2Params
{
    SolPubkey market;
    SolPubkey bids;
    SolPubkey asks;
    SolPubkey event_queue;
    SolPubkey open_orders;
    SolPubkey owner;
    Side side;
    uint8_t order_id[16];
    // uint64_t open_orders_slot;
    SolPubkey program_id;
};

// struct CancelOrderByClient
// {
//     uint64_t client_id;
// };

// struct CancelOrderV2
// {
//     uint32_t side;
//     uint8_t order_id[16];
// };

// struct CancelOrderByClientV2
// {
//     uint64_t client_id;
// };

// struct InstructionLayout
// {
//     const uint8_t version = VERSION;
//     uint32_t instruction_type;
// };

// struct InstructionLayoutNewOrderV3 : InstructionLayout
// {
//     NewOrderV3 args;
// };

// struct InstructionLayoutCancelOrderV2 : InstructionLayout
// {
//     CancelOrderV2 args;
// };


#define SIZE_HASH 32
#define SIZE_SIGNATURE 32
/**
 * Signature
 */
// typedef struct {
//     uint8_t x[SIZE_SIGNATURE];
// } Signature;

/**
 * Hash
 */
typedef struct {
    uint8_t x[SIZE_HASH];
} Hash;