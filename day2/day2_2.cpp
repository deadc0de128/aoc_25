#include <iostream>
#include <fstream>
#include <charconv>
#include <string_view>
#include <cmath>
#include <cassert>
#include <cstdint>

using namespace std;

uint64_t exp10(int power)
{
    uint64_t ret = 1;
    for (int i = 0; i < power; ++i)
        ret *= 10;
    return ret;
}

/*bool is_invalid(uint64_t n)
{
    auto len10 = (int)ceil(log10((double)n));
    if (len10 % 2) return false;
    auto half = exp10(len10/2);
    return n % half == n / half;
}*/

bool is_invalid(uint64_t n)
{
    auto len10 = (int)ceil(log10((double)n));
    if (exp10(len10) == n)
        len10++;

    bool ret = false;

    for (int i = 1; i <= (len10+1)/2; ++i)
    {
        if (len10 % i) continue;
        auto divd = exp10(i);
        auto number = n;
        uint64_t part = number % divd;
        number /= divd;
        
        bool equal = false;

        for (int j = 2; j <= (len10 / i); ++j)
        {
            if (number % divd != part)
            {
                equal = false;
                break;
            }

            number /= divd;

            equal = true;
        }

        if (equal) 
        {
            ret = true;
            break;
        }
    }

    return ret;
}

int main(int argc, char** argv)
{
    assert(!is_invalid(100));
    assert(is_invalid(11));
    assert(is_invalid(22));
    assert(is_invalid(99));
    assert(is_invalid(111));
    assert(is_invalid(999));
    assert(is_invalid(1010));
    assert(is_invalid(123123));
    assert(!is_invalid(1024));

    ifstream f(argv[1]);

    if (!f.is_open()) return -1;

    uint64_t sum = 0;

    string line;
    while (getline(f, line, ','))
    {
//        std::cout << line << '\n';
        auto pos = line.find('-');
        if (pos == string::npos)
        {
            cerr << " - not found\n";
            return -1;
        }
        const char* start = line.c_str();
        const char* end = line.c_str() + pos;
        uint64_t r_start, r_end;
        auto ret = from_chars(start, end, r_start);
        if (ret.ec != errc())
        {
            cerr << "Cannot convert \"" << string_view(start, end) << "\": "
                 << make_error_code(ret.ec).message() << '\n';
            return -1;
        }
        start = end + 1;
        end = line.c_str() + line.size();
        ret = from_chars(start, end, r_end);
        if (ret.ec != errc())
        {
            cerr << "Cannot convert \"" << string_view(start, end) << "\": "
                 << make_error_code(ret.ec).message() << '\n';
            return -1;
        }
//        std::cout << r_start << " " << r_end << '\n';

        auto i = r_start;

        for (;;)
        {
            if (is_invalid(i))
            {
//                std::cout << i << '\n';
                sum += i;
            }
            if (i == r_end) break;
            i++;
        }
    }

    std::cout << sum << '\n';

    return 0;
}

