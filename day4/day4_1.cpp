#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int main(int argc, char** argv)
{
    fstream f(argv[1]);

    if (!f.is_open()) return -1;

    vector<vector<char>> field;

    string str;
    while (getline(f, str))
    {
        field.emplace_back(str.begin(), str.end());
    }

    const ssize_t x_size = field[0].size();
    const ssize_t y_size = field.size();

    int accessible = 0;

    for (ssize_t y = 0; y < y_size; ++y)
        for (ssize_t x = 0; x < x_size; ++x)
        {
            if (field[y][x] != '@') continue;

            int count = 0;

            for (ssize_t cy = std::max(0z, y - 1); cy <= std::min(y_size - 1, y + 1); ++cy)
                for (ssize_t cx = std::max(0z, x - 1); cx <= std::min(x_size - 1, x + 1); ++cx)
                {
                    if (cy == y && cx == x) continue;
                    if (field[cy][cx] == '@')
                        count++;
                }

            accessible += (count < 4 ? 1 : 0);
        }

    cout << accessible << '\n';

    return 0;
}
