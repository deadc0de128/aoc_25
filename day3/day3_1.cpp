#include <iostream>
#include <fstream>
#include <string_view>
#include <algorithm>

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

    int64_t sum = 0;

    std::string line;
    while (getline(f, line))
    {
        auto [first, first_it] = get_biggest_num(string_view(line.c_str(), line.c_str() + line.size() - 1));
        auto [second, second_it] = get_biggest_num(string_view(line.c_str() + first_it + 1, line.c_str() + line.size()));
        std::cout << first << " " << second << '\n';
        sum += first * 10 + second;
    }

    std::cout << sum << '\n';

    return 0;
}
