#include "base58.h"

std::string base58_decode(const std::string& vec_, Alphabet alph)
{
    std::map<char, uint8_t> alphabet_map;
    switch (alph)
    {
    case Alphabet::Bitcoin: 
        alphabet_map = BITCOIN_ALPHABET_MAP;
        break;
    }

    auto vec = vec_;
    vec.erase(
        vec.begin(),
        std::find_if_not(begin(vec), end(vec), [&](char chr) 
        {
            return chr == alphabet_map.begin()->first;
        })
    );

    mpz_class decimal = 0;
    for (const auto& chr : vec) {
        decimal = decimal * 58 + alphabet_map.at(chr);
    }    

    std::string res (vec_.size() - vec.size(), 0);
    while(decimal > 0) {
        mpz_class r;
        mpz_tdiv_qr(decimal.get_mpz_t(), r.get_mpz_t(), decimal.get_mpz_t(), mpz_class(256).get_mpz_t());
        res.push_back(r.get_si());
    }
    std::reverse(res.begin(), res.end());
    return res;
};

std::string base58_encode(const std::string& vec_, Alphabet alph)
{
    std::string alphabet;
    switch (alph)
    {
    case Alphabet::Bitcoin: 
        alphabet = BITCOIN_ALPHABET;
        break;
    }

    auto vec = vec_;
    vec.erase(
        vec.begin(),
        std::find_if_not(begin(vec), end(vec), [](uint8_t chr) 
        {
            return chr == 0;
        })
    );

    mpz_class decimal;
    mpz_import(decimal.get_mpz_t(), vec.size(), 1, sizeof(vec[0]), 0, 0, vec.data());
    
    std::string res (vec_.size() - vec.size(), alphabet[0]);
    while(decimal > 0) {
        mpz_class r;
        mpz_tdiv_qr(decimal.get_mpz_t(), r.get_mpz_t(), decimal.get_mpz_t(), mpz_class(58).get_mpz_t());
        res.push_back(alphabet[r.get_si()]);
    }
    std::reverse(res.begin(), res.end());
    return res;
}