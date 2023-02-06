#pragma once
#include <stdint.h>

namespace solana
{
    enum struct InstructionType : uint8_t
    {
        IINITIALIZE_MINT = 0,
        INITIALIZE_ACCOUNT = 1,
        CLOSE_ACCOUNT = 9
    };
} // namespace solana
