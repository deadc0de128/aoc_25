#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <numeric>
#include <ctype.h>

using namespace std;

int main(int argc, char** argv)
{
    ifstream f(argv[1]);

    if (!f.is_open()) return -1;

    vector<vector<char>> matrix;

    int column = 0;

    string line;
    while (getline(f, line))
    {
        if (line.empty()) return -1;
        matrix.emplace_back(line.begin(), line.end());
    }

    if (matrix.empty()) return -1;

    const size_t ncol = matrix[0].size();
    const size_t nrow = matrix.size();

    uint64_t sum = 0, part = 0;

    char op;

    for (size_t i = 0; i < ncol; ++i)
    {
        uint32_t num = 0;
        bool blank = true;
        for (size_t j = 0; j < nrow - 1; ++j)
        {
            if (matrix[j][i] == ' ') continue;
            num *= 10;
            num += (matrix[j][i] - '0');
            blank = false;
        }

        if (matrix[nrow - 1][i] == '*')
        {
            op = matrix[nrow - 1][i];
            part = 1;
        }

        if (matrix[nrow - 1][i] == '+')
        {
            op = matrix[nrow - 1][i];
            part = 0;
        }

        if (!blank)
        {
            cout << num << '\n';

            if (op == '+')
                part += num;
            else
                part *= num;
        }

        if (blank || i == ncol - 1)
        {
            cout << op << " -> " << part << '\n';
            sum += part;
        }
    }

    cout << sum << '\n';

    return 0;
}
