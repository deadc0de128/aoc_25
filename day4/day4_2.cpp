#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int main(int argc, char** argv)
{
    fstream f(argv[1]);

    if (!f.is_open()) return -1;

    vector<vector<char>> field;
    vector<vector<char>> field_new;
    string str;
    while (getline(f, str))
    {
        field.emplace_back(str.begin(), str.end());
    }

    field_new = field;

    const ssize_t x_size = field[0].size();
    const ssize_t y_size = field.size();

    int accessible = 0;
    int cycle_accessible;

    do
    {
        cycle_accessible = 0;

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

                if (count < 4)
                {
                    field_new[y][x] = '.';
                    cycle_accessible++;
                }
            }

        field = field_new;
        accessible += cycle_accessible;
    }
    while (cycle_accessible);

    cout << accessible << '\n';

    return 0;
}
