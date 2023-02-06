#include "Hash.hpp"


namespace solana
{
    Hash::Hash() : hash_str(), hash_b(bytes(0))
    {}

    Hash::Hash(const string& hash) : hash_str(hash), hash_b(_from_base58(hash_str))
    {}

    Hash::Hash(const bytes& hash)
    {
        if (hash.size() != SIZE_HASH) {
            throw -1;
        }

        hash_b = hash;
        hash_str = base58_encode(string((char*)hash.data(), hash.size()));
    } 

    Hash::Hash(const Hash& other) : hash_str(other.hash_str), hash_b(other.hash_b)
    {}

    Hash::Hash(Hash&& other) : hash_str(other.hash_str), hash_b(other.hash_b)
    {
        other.hash_b = bytes(0);
        other.hash_str = "";
    }

    Hash::~Hash()
    {}

    void Hash::from_base58(const string& hash)
    {
        hash_str = hash;
        hash_b = _from_base58(hash_str);
    }

    Hash::bytes Hash::_from_base58(const string& hash)
    {
        auto decoded_key = base58_decode(hash);
        auto res = bytes(SIZE_HASH);
        memcpy(res.data(), decoded_key.data(), SIZE_HASH);
        return res;
    }

    bool operator==(const Hash &k1, const Hash &k2)
    {
        if (k1.hash_b.empty() || k2.hash_b.empty()) {
            throw -1;
        }

        return !memcmp(k1.hash_b.data(), k2.hash_b.data(), SIZE_HASH);
    }

    bool operator==(const Hash &k1, const std::string &k2)
    {
        if (k1.hash_str.size() == 0) {
            throw -1;
        }

        return  k1.hash_str == k2;
    }

    Hash& Hash::operator= (const Hash &hash)
    {
        if (this == &hash)
            return *this;

        hash_str = hash.hash_str;
        hash_b = hash.hash_b;
        return *this;
    }

    Hash& Hash::operator= (Hash &&hash)
    {
        if (this == &hash)
            return *this;

        hash_str = hash.hash_str;
        hash_b = hash.hash_b;

        hash.hash_b = bytes(0);
        hash.hash_str = "";

        return *this;
    }
}