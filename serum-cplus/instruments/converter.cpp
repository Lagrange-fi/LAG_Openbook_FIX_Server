#include "converter.hpp"

__uint128_t atouint128(const std::string& in)
{
    __uint128_t res = 0;
    for (const char& c : in)
    {
        if (not std::isdigit(c)) 
            throw std::string("Non-numeric character: ");
        res *= 10;
        res += c - '0';
    }
    return res;
}

std::string uint128toa(__uint128_t in)
{
    std::string res = std::string();

    while (in) 
    {
        char c = in % 10 + '0';
        res.insert(res.begin(), c);
        in /= 10;
    }

    if (!res.size())
        return "0";
    return res;
}