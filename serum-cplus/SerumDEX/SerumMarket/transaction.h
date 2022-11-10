#pragma once
#include "structs.h"
#include "sol_sdk/cpi.h"
#include <vector>
#include <string>


class Transaction {
private:
    typedef std::string string;
    typedef std::vector<string> Signatures;
    typedef std::vector<SolPubkey> Keys;
    typedef std::vector<SolInstruction> Instructions;

    typedef struct {
        uint8_t num_required_signatures {0};
        uint8_t num_readonly_signed_accounts {0};
        uint8_t num_readonly_unsigned_accounts {0};
        uint8_t num_accounts {0};
    } MessageHeader;

    typedef struct {
        MessageHeader header;
        std::vector<uint8_t> account_keys;
        Hash recent_blockhash;
        std::vector<uint8_t> data;
    } _Message;

    Signatures signatures;
    _Message _message;
    Instructions instructions;
    string serialized_message;

    // string get_message_for_sign();
    bool compare_key(const SolPubkey&, const SolPubkey&);
    void add_keys(std::vector<uint8_t>&, const std::vector<SolPubkey*>, size_t);
    uint8_t index_of_pub_key(const SolPubkey& , const std::vector<uint8_t>&);
    std::vector<uint8_t> compile_instruction(const SolInstruction&, const std::vector<uint8_t>&);
    std::vector<uint8_t> get_message_from_instructions(const Instructions&, const std::vector<uint8_t>&);
public:
    string get_message_for_sign();
    // void add_account_key(SolPubkey);
    void add_instruction(const SolInstruction&);
    void set_recent_blockhash(const string&);
    void sign(const SolKeyPair&);
    string serialize();
};