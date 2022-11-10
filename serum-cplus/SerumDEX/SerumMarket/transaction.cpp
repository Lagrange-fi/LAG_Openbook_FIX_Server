#include "transaction.h"
#include "instruments.h"
#include <algorithm>
#include <functional>
#include <numeric>
#include <string.h>
#include <iostream>
#include <openssl/x509.h>
#include <cryptopp/xed25519.h>
#include <cryptopp/osrng.h>


std::string to_hex(const uint8_t* vec, size_t len)
{
    std::string map = "0123456789abcdef";
    std::string res = "";
    for (size_t i = 0; i < len; i++, vec++) {
        res.push_back(map[(*vec >> 4) & 0xf]);
        res.push_back(map[*vec & 0xf]);
    }

    return res;
}


void Transaction::add_instruction(const SolInstruction& instruction)
{
    instructions.push_back(instruction);
}

void Transaction::set_recent_blockhash(const string& blockhash)
{
    _message.recent_blockhash = base58str_to_hash(blockhash);
}

std::string Transaction::serialize()
{
    string msg = "";

    //TODO compact array
    msg.push_back(static_cast<char>(signatures.size()));

    for (const auto& sign: signatures)
        msg.insert(msg.end(), sign.begin(), sign.end());

    msg.insert(msg.end(), serialized_message.begin(), serialized_message.end());
    return msg;
}

void Transaction::sign(const SolKeyPair& private_key)
{
    
    CryptoPP::byte raw_private_key[SIZE_PUBKEY];
    CryptoPP::byte raw_public_key[SIZE_PUBKEY];
    memcpy(raw_private_key, &(private_key.x), SIZE_PUBKEY);
    memcpy(raw_public_key, &(private_key.x) + SIZE_PUBKEY, SIZE_PUBKEY);
    CryptoPP::ed25519::Signer signer(raw_public_key, raw_private_key);

    if (serialized_message.size() == 0) 
        serialized_message = get_message_for_sign();
    string signature;

    CryptoPP::AutoSeededRandomPool prng;
    // Determine maximum signature size
    size_t siglen = signer.MaxSignatureLength();
    signature.resize(siglen);

    // Sign, and trim signature to actual size
    siglen = signer.SignMessage(prng, (const CryptoPP::byte*)&serialized_message[0], serialized_message.size(), (CryptoPP::byte*)&signature[0]);
    signature.resize(siglen);

    signatures.push_back(signature);
}

std::string Transaction::get_message_for_sign()
{
    // size_t datas_len = std::accumulate(
    //     instructions.begin(), 
    //     instructions.end(), 
    //     0, 
    //     [](size_t sum, const SolInstruction& a) {return sum + a.data_len;}
    // );

    // _message.data = std::vector<uint8_t>(datas_len, 0);
    // size_t occupied = 0;

    std::vector<SolPubkey*> signed_writable_accounts;
    std::vector<SolPubkey*> signed_accounts;
    std::vector<SolPubkey*> writable_accounts;
    std::vector<SolPubkey*> other_accounts;

    for (auto instr: instructions) {
        _message.header.num_accounts+=instr.account_len;
        auto tmp = instr.accounts;
        for (size_t i = 0; i < instr.account_len; i++, tmp++) {
            if (tmp->is_signer) {
                ++_message.header.num_required_signatures;
                if (tmp->is_writable) {
                    signed_writable_accounts.push_back(&tmp->pubkey);
                }
                else {
                    ++_message.header.num_readonly_signed_accounts;
                    signed_accounts.push_back(&tmp->pubkey);
                }
            }
            else {
                if (tmp->is_writable) {
                    writable_accounts.push_back(&tmp->pubkey);
                }
                else {
                    ++_message.header.num_readonly_unsigned_accounts;
                    other_accounts.push_back(&tmp->pubkey);
                }
            }
        }
        // memcpy(_message.data.data() + occupied, instr.data, instr.data_len);
        // occupied += instr.data_len;
    }

    // TODO add all program id
    other_accounts.push_back(&instructions[0].program_id);
    // _message.header.num_accounts = _message.header.num_required_signatures 
    //     + _message.header.num_readonly_signed_accounts 
    //     + _message.header.num_readonly_unsigned_accounts;
    _message.header.num_readonly_unsigned_accounts++;
    _message.header.num_accounts++;

    _message.account_keys = std::vector<uint8_t>(SIZE_PUBKEY * _message.header.num_accounts, 0);

    size_t shift = 0;
    add_keys(_message.account_keys, signed_writable_accounts, shift);
    shift += signed_writable_accounts.size() * SIZE_PUBKEY;

    add_keys(_message.account_keys, signed_accounts, shift);
    shift += signed_accounts.size() * SIZE_PUBKEY;

    add_keys(_message.account_keys, writable_accounts, shift);
    shift += writable_accounts.size() * SIZE_PUBKEY;

    add_keys(_message.account_keys, other_accounts, shift);
    shift += other_accounts.size() * SIZE_PUBKEY;

    auto compiled_instructions = get_message_from_instructions(instructions, _message.account_keys);

    size_t size = 4+_message.account_keys.size()+SIZE_HASH+compiled_instructions.size();
    uint8_t msg[size];

    shift = 0;
    memcpy(msg, &_message.header, 4); 
    shift += 4;
    memcpy(msg + shift, _message.account_keys.data(), _message.account_keys.size()); 
    shift += _message.account_keys.size();
    memcpy(msg + shift, &_message.recent_blockhash, SIZE_HASH); 
    shift += SIZE_HASH;
    memcpy(msg + shift, compiled_instructions.data(), compiled_instructions.size()); 
    shift += compiled_instructions.size();

    // auto tt = to_hex(msg, size);
    // std::cout << tt << std::endl;
    return string((char*)msg, size);
}

std::vector<uint8_t> Transaction::get_message_from_instructions(const Instructions &instructions, const std::vector<uint8_t> &keys)
{
    std::vector<uint8_t> msg = std::vector<uint8_t>();
    // TODO Compact-Array Format
    msg.push_back(instructions.size());
    for (const auto& instr: instructions) {
        auto compiled_instruction = compile_instruction(instr, keys);
        msg.insert(msg.end(), compiled_instruction.begin(), compiled_instruction.end());
    }

    return msg;
}


std::vector<uint8_t> Transaction::compile_instruction(const SolInstruction& instr, const std::vector<uint8_t>& keys)
{
    // TODO Compact-Array Format

    uint64_t size = 1 // the program id shift
    + 1 // num_accounts
    + instr.account_len // accounts shift
    + 1 // data size
    + instr.data_len;
    
    std::vector<uint8_t> compiled_instruction = std::vector<uint8_t>(size);

    uint64_t shift = 0;
    memset(compiled_instruction.data(), index_of_pub_key(instr.program_id, keys), 1); 
    shift += 1;

    // TODO account_len into a Compact-Array Format
    memset(compiled_instruction.data() + shift, static_cast<uint8_t>(instr.account_len), 1); 
    shift += 1;

    for (uint64_t  i = 0; i < instr.account_len; i++) {
        memset(compiled_instruction.data() + shift, index_of_pub_key(instr.accounts[i].pubkey, keys), 1);
        shift += 1;
    }

    // TODO data_size into a Compact-Array Format
    memset(compiled_instruction.data() + shift, instr.data_len, 1);
    shift += 1;

    memcpy(compiled_instruction.data() + shift, instr.data, instr.data_len); 
    shift += instr.data_len;

    return compiled_instruction;
}

uint8_t Transaction::index_of_pub_key(const SolPubkey& pubkey, const std::vector<uint8_t>& keys)
{
    uint8_t count = static_cast<uint8_t>(keys.size() / SIZE_PUBKEY);
    for (uint8_t i = 0; i < count; i++) {
        if (memcmp(&pubkey, keys.data() + i*SIZE_PUBKEY, SIZE_PUBKEY) == 0) {
            return i;
        }
    }
    return -1;
}

void Transaction::add_keys(std::vector<uint8_t>& data, const std::vector<SolPubkey*> new_keys, size_t shift)
{
    for (auto key : new_keys) {
        memcpy(data.data() + shift, key, SIZE_PUBKEY);
        shift += SIZE_PUBKEY;
    }
}
    