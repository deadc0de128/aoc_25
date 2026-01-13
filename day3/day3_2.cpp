#include <iostream>
#include <fstream>
#include <string_view>
#include <algorithm>
#include <cstdint>

using namespace std;

std::pair<int, ptrdiff_t> get_biggest_num(std::string_view s)
{
    char max_num = '0';
    for (char c : s)
    {
        max_num = std::max(max_num, c);
    }

    return std::make_pair(max_num - '0', std::distance(s.begin(), find(s.begin(), s.end(), max_num)));
}

int main(int argc, char** argv)
{
    ifstream f(argv[1]);

    if (!f.is_open()) return -1;

    uint64_t sum = 0;

    std::string line;
    while (getline(f, line))
    {
        uint64_t line_sum = 0;
        const char* start = line.c_str();

        for (int i = 1; i <= 12; ++i)
        {
            auto [num, num_ptr] = get_biggest_num(string_view(start, line.c_str() + line.size() - (12 - i)));
            start = start + num_ptr + 1;
            line_sum *= 10;
            line_sum += num;
        }

        std::cout << line_sum << '\n';

        sum += line_sum;
    }

    std::cout << sum << '\n';

    return 0;
}
