#pragma once
#include <vector>
#include <string>
#include <string.h>
#include <base58/base58.h>

namespace solana
{
    #define SIZE_HASH 32

    class Hash 
    {
        typedef std::string string;
        typedef uint8_t byte;
        typedef std::vector<byte> bytes;


        string hash_str;
        bytes hash_b;

        bytes _from_base58(const string&);
    public:
        Hash();
        Hash(const string&);
        Hash(const bytes&);
        Hash(const Hash&);
        Hash(Hash&&);
    

        void from_base58(const string&);
        const uint8_t* data() {return hash_b.data();}
        size_t size() {return hash_b.size();}

        friend bool operator==(const Hash&, const Hash&);
        friend bool operator==(const Hash&, const string&);
        Hash& operator=(const Hash&);
        Hash& operator=(Hash&&);
        ~Hash();
    };
} // namespace solana

