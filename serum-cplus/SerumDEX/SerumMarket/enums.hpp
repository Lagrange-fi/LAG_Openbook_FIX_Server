#pragma once 
#include <stdint.h>

enum Side : uint32_t
{
    BUY = 0,
    SELL = 1,
};

enum OrderType : uint32_t
{
    LIMIT = 0,
    IOC = 1,
    POST_ONLY = 2
};

enum SelfTradeBehavior : uint32_t
{
    DECREMENT_TAKE = 0,
    CANCEL_PROVIDE = 1,
    ABORT_TRANSACTION = 2
};

enum InstructionType
{
    INITIALIZE_MARKET = 0,
    NEW_ORDER = 1,
    MATCH_ORDER = 2,
    CONSUME_EVENTS = 3,
    CANCEL_ORDER = 4,
    SETTLE_FUNDS = 5,
    CANCEL_ORDER_BY_CLIENT_ID = 6,
    NEW_ORDER_V3 = 10,
    CANCEL_ORDER_V2 = 11,
    CANCEL_ORDER_BY_CLIENT_ID_V2 = 12
};