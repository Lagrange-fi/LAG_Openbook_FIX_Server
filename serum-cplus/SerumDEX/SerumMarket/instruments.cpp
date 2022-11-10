#include "instruments.h"
#include <base58/base58.h>

std::vector<uint8_t> base58str_to_bytes(const std::string& str_key, size_t len)
{
    auto _key = base58_decode(str_key);
    if (_key.size() > len) {
        throw std::exception();
    }

    int i = len - 1;
    // SolKeyPair key;
    std::vector<uint8_t> res (len);
    std::for_each(_key.rbegin(), _key.rend(), [&i, &res] (uint8_t byte) 
    {
        res[i--] = byte;
    });
    return res;
}

SolPubkey base58str_to_pubkey(const std::string& str_key)
{
    auto key = SolPubkey();
    memcpy(key.x, static_cast<void*>(base58str_to_bytes(str_key, SIZE_PUBKEY).data()), SIZE_PUBKEY);
    return key;
}

SolKeyPair base58str_to_keypair(const std::string& str_key)
{
    auto key = SolKeyPair();
    memcpy(key.x, static_cast<void*>(base58str_to_bytes(str_key, SIZE_KEYPAIR).data()), SIZE_KEYPAIR);
    return key;
}

Hash base58str_to_hash(const std::string& str_key)
{
    auto hash = Hash();
    memcpy(hash.x, static_cast<void*>(base58str_to_bytes(str_key, SIZE_HASH).data()), SIZE_HASH);
    return hash;
}