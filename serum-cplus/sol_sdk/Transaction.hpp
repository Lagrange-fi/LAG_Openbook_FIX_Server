#pragma once
#include "PublicKey.hpp"
#include "Keypair.hpp"
#include "Hash.hpp"
#include "Instruction.hpp"
#include <vector>
#include <string>
#include <base58/base58.h>
#include <algorithm>
#include <functional>
#include <numeric>
#include <string.h>
#include <cryptopp/xed25519.h>
#include <cryptopp/osrng.h>
#include <set>

namespace solana
{
    class Transaction {
    public: 
        typedef std::vector<Keypair> Signers;

    private:
        typedef std::string string;
        typedef std::vector<string> Signatures;
        typedef std::vector<PublicKey> Keys;
        typedef std::vector<Instruction> Instructions;
        typedef std::vector<uint8_t> bytes;

        typedef struct {
            uint8_t num_required_signatures {0};
            uint8_t num_readonly_signed_accounts {0};
            uint8_t num_readonly_unsigned_accounts {0};
            uint8_t num_accounts {0};
        } MessageHeader;

        typedef struct {
            MessageHeader header;
            bytes account_keys;
            Hash recent_blockhash;
            bytes data;
        } _Message;

        Signatures signatures;
        _Message _message;
        Instructions instructions;
        string serialized_message;

        // string get_message_for_sign();
        bool compare_key(const PublicKey&, const PublicKey&);
        void add_keys(bytes&, const std::vector< PublicKey>&, size_t);
        uint8_t index_of_pub_key(const PublicKey& , const bytes&);
        bytes compile_instruction(const Instruction&, const bytes&);
        bytes get_message_from_instructions(const Instructions&, const bytes&);
        string get_message_for_sign();
        void _sign(const Keypair&);
    public:
        // void add_account_key(SolPubkey);
        void add_instruction(const Instruction&);
        void set_recent_blockhash(const string&);
        void sign(const Signers&);
        string serialize() const;
    };
} // namespace solana