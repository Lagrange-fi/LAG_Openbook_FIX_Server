#pragma once 
#include <string>
#include <vector>
#include <algorithm>
#include "alphabets.h"
#include <gmpxx.h>

std::string base58_decode(const std::string& vec_, const std::map<char, uint8_t>& alphabet_map = BITCOIN_ALPHABET_MAP);

// class Base58
// {
// public:
//     static std::vector<uint8_t> decode(const std::string& vec_, const std::map<char, uint8_t>& alphabet_map = BITCOIN_ALPHABET_MAP)
//     {
//         auto vec = vec_;
//         vec.erase(
//             vec.begin(),
//             std::find_if_not(begin(vec), end(vec), [&](char chr) 
//             {
//                 return chr == alphabet_map.begin()->first;
//             })
//         );

//         mpz_class decimal = 0;
//         for (const auto& chr : vec) {
//             decimal = decimal * 58 + alphabet_map.at(chr);
//         }        
//         std::vector<uint8_t> res (vec_.size() - vec.size(), 0);
//         while(decimal > 0) {
//             mpz_class r;
//             mpz_tdiv_qr(decimal.get_mpz_t(), r.get_mpz_t(), decimal.get_mpz_t(), mpz_class(256).get_mpz_t());
//             res.push_back(r.get_si());
//         }
//         std::reverse(res.begin(), res.end());
//         return res;
//     };
//     // static std::vector<uint8_t> encode(const std::string& vec);
//     //  {
//     //     std::vector<uint8_t> result{};

//     //     uint64_t curr = bitcoin_alphabet_map[vec[12]];
//     //     std::for_each(vec.rbegin()+1, vec.rend(), [&](const uint8_t chr) 
//     //     { 
//     //         curr = curr + bitcoin_alphabet_map[chr] * 58;
//     //         result.push_back(curr & 0xff);
//     //         curr >>= 8;
//     //     });


//     //     // for (auto chr = vec.rbegin(); chr < vec.rend(); chr++) {
//     //     //     curr = (curr + bitcoin_alphabet_map[*chr]) * 58;
//     //     //     result.push_back(curr & 0xff);
//     //     //     curr >>= 8;
//     //     // }

//     //     return result;
//     // }
// };

// 100011010110