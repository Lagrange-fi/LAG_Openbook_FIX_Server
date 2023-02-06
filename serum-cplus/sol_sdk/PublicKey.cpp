#include "PublicKey.hpp"

namespace solana
{
    PublicKey::PublicKey() : key_str(), key_b(bytes(0))
    {}

    PublicKey::PublicKey(const string& key) : key_str(key), key_b(_from_base58(key_str))
    {}

    PublicKey::PublicKey(const bytes& key)
    {
        if (key.size() != SIZE_PUBKEY) {
            throw -1;
        }

        key_b = key;
        key_str = _to_base58(key_b);
    } 

    PublicKey::PublicKey(const PublicKey& other) : key_str(other.key_str), key_b(other.key_b)
    {}

    PublicKey::PublicKey(PublicKey&& other) : key_str(std::move(other.key_str)), key_b(std::move(other.key_b))
    {
        // other.key_b = bytes(0);
        // other.key_str = "";
    }

    PublicKey::PublicKey(const byte key[SIZE_PUBKEY]) : key_b(key, key + SIZE_PUBKEY)
    {
        key_str = _to_base58(key_b);
    }

    PublicKey::~PublicKey()
    {}

    void PublicKey::from_base58(const string& key)
    {
        key_str = key;
        key_b = _from_base58(key_str);
    }

    PublicKey::bytes PublicKey::_from_base58(const string& key)
    {
        auto decoded_key = base58_decode(key);
        auto res = bytes(SIZE_PUBKEY);
        // memcpy(res.data(), decoded_key.data(), SIZE_PUBKEY);
        std::copy(decoded_key.begin(), decoded_key.end(), res.begin());
        return res;
    }

    PublicKey::string PublicKey::_to_base58(const bytes& key)
    {
        return base58_encode(string((char*)key.data(), key.size()));
    }

    bool operator==(const PublicKey &k1, const PublicKey &k2)
    {
        if (k1.key_b.empty() || k2.key_b.empty()) {
            throw -1;
        }

        return  !memcmp(k1.key_b.data(), k2.key_b.data(), SIZE_PUBKEY);
    }

    bool operator==(const PublicKey &k1, const std::string &k2)
    {
        if (k1.key_str.empty()) {
            throw -1;
        }

        return  k1.key_str == k2;
    }

    PublicKey& PublicKey::operator= (const PublicKey &key)
    {
        if (this == &key)
            return *this;

        key_str = key.key_str;
        key_b = key.key_b;
        return *this;
    }

    PublicKey& PublicKey::operator= (PublicKey &&key)
    {
        if (this == &key)
            return *this;

        key_str = key.key_str;
        key_b = key.key_b;

        key.key_b = bytes(0);
        key.key_str = "";

        return *this;
    }
} // namespace solana