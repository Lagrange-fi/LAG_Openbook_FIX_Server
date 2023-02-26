#include "Keypair.hpp"

namespace solana
{
    Keypair::Keypair()
    {
        _generate_new_key();
    }

    Keypair::Keypair(const string& key) : key_str(key), key_b(_from_base58(key_str))
    {}

    Keypair::Keypair(const bytes& key)
    {
        if (key.size() != SIZE_KEYPAIR) {
            throw -1;
        }

        key_b = key;
        key_str = _to_base58(key_b);
    } 

    Keypair::Keypair(const Keypair& other) : key_str(other.key_str), key_b(other.key_b)
    {}

    Keypair::Keypair(Keypair&& other) : key_str(other.key_str), key_b(other.key_b)
    {
        other.key_b = bytes(0);
        other.key_str = "";
    }

    Keypair:: Keypair(const byte key[SIZE_KEYPAIR]) : key_b(key, key + SIZE_KEYPAIR)
    {
        key_str = _to_base58(key_b);
    }

    Keypair::~Keypair()
    {}

    void Keypair::from_base58(const string& key)
    {
        key_str = key;
        key_b = _from_base58(key_str);
    }

    PublicKey Keypair::get_pubkey() const
    {
        auto vec = bytes(32);
        std::copy(key_b.end() - SIZE_PUBKEY, key_b.end(), vec.begin());
        return PublicKey(vec);
    }

    void Keypair::_generate_new_key()
    {

        auto set_bytes = [](bytes& dest, const CryptoPP::Integer& src, size_t shift = 0) {
            string hex_;
            std::stringstream temp;
            temp << std::hex << src;
            temp >> hex_;
            hex_.pop_back();
            while (hex_.size() < 64)
                hex_.push_back('0');
            std::string hash = boost::algorithm::unhex(hex_);
            std::reverse(hash.begin(), hash.end());
            std::copy(hash.begin(), hash.end(), dest.begin() + shift);
        };

        key_b = bytes(CryptoPP::x25519::SECRET_KEYLENGTH + CryptoPP::x25519::PUBLIC_KEYLENGTH);

        CryptoPP::AutoSeededRandomPool prng;
        CryptoPP::ed25519::Signer signer;
        signer.AccessPrivateKey().GenerateRandom(prng);
        const CryptoPP::ed25519PrivateKey& x = dynamic_cast<const CryptoPP::ed25519PrivateKey&> (signer.GetPrivateKey());
        set_bytes(key_b, x.GetPrivateExponent());
        
        CryptoPP::ed25519::Verifier ver(signer);
        const CryptoPP::ed25519PublicKey& y = dynamic_cast<const CryptoPP::ed25519PublicKey&> (ver.GetPublicKey());
        set_bytes(key_b, y.GetPublicElement(), CryptoPP::x25519::SECRET_KEYLENGTH);

        key_str = _to_base58(key_b);
    }

    Keypair::bytes Keypair::_from_base58(const string& key)
    {
        auto decoded_key = base58_decode(key);
        auto res = bytes(SIZE_KEYPAIR);
        memcpy(res.data(), decoded_key.data(), SIZE_KEYPAIR);
        return res;
    }

    Keypair::string Keypair::_to_base58(const bytes& key)
    {
        return base58_encode( string((char*)key.data(), key.size()) );
    }

    bool operator==(const Keypair &k1, const Keypair &k2)
    {
        if (k1.key_b.empty() || k2.key_b.empty()) {
            throw -1;
        }

        return  !memcmp(k1.key_b.data(), k2.key_b.data(), SIZE_KEYPAIR);
    }

    bool operator==(const Keypair &k1, const std::string &k2)
    {
        if (k1.key_str.empty()) {
            throw -1;
        }

        return  k1.key_str == k2;
    }

    Keypair& Keypair::operator= (const Keypair &key)
    {
        if (this == &key)
            return *this;

        key_str = key.key_str;
        key_b = key.key_b;
        return *this;
    }

    Keypair& Keypair::operator= (Keypair &&key)
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