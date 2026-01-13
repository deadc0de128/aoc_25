#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <numeric>
#include <ctype.h>
#include <sstream>

using namespace std;

int main(int argc, char** argv)
{
    ifstream f(argv[1]);

    if (!f.is_open()) return -1;

    vector<vector<int>> values;

    int column = 0;

    string line;
    while (getline(f, line))
    {
        cout << line << '\n';
        if (line.empty()) return -1;
        if (line[0] != ' ' && !isdigit(line[0]))
            break;

        istringstream s(std::move(line));

        while (s)
        {
            int v;
            s >> v;

            if (values.size() <= column)
                values.resize(values.size() + 1);

            values[column].push_back(v);

            column++;
        }

        column = 0;
    }

    if (values.empty()) return -1;

    for (size_t row = 0; row < values[0].size(); ++row)
    {
        for (size_t col = 0; col < values.size(); ++col)
        {
            cout << values[col][row] << " ";
        }
        cout << '\n';
    }

    column = 0;

    uint64_t sum = 0;

    for (char op : line)
    {
        if (op == '*')
        {
            uint64_t acc = 1;
            acc = accumulate(values[column].begin(), values[column].end(), acc, std::multiplies{});
            sum += acc;
        }
        else if (op == '+')
        {
            uint64_t acc = 0;
            acc = accumulate(values[column].begin(), values[column].end(), acc);
            sum += acc;
        }
        else
            continue;

        column++;
    }

    cout << sum << '\n';

    return 0;
}
