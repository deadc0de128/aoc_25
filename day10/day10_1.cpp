
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <limits>

using namespace std;

using state = uint32_t;
using button = uint32_t;

bool get_bit(uint32_t v, int bit)
{
    return (v & (1 << bit)) != 0;
}

void set_bit(uint32_t& v, int bit)
{
    v |= (1 << bit);
}

void flip_bit(uint32_t& v, int bit)
{
    v ^= (1 << bit);
}

vector<uint32_t> get_numbers(std::string data)
{
    vector<uint32_t> ret;

    istringstream s(std::move(data));
    while (s)
    {
        uint32_t n;
        s >> n;
        s.ignore(1, ',');
        ret.push_back(n);
    }

    return ret;
}

struct input
{
    state target;
    vector<button> buttons;
};

ostream& operator<<(ostream& s, const input& in)
{
    s << '[';
    for (int i = 0; i < 10/*sizeof(state)*8*/; ++i)
    {
        if (get_bit(in.target, i)) s << '#'; else s << '.';
    }
    s << "] ";
    for (auto b : in.buttons)
    {
        s << '(';
        for (int i = 0; i < sizeof(button)*8; ++i)
            if (get_bit(b, i))
                s << i << ", ";
        s << ") ";
    }

    return s;
}

unsigned min_presses(unordered_map<state, unsigned>& graph, const vector<button>& buttons, state target, state current = 0, unsigned path_len = 0)
{
    auto it_c = graph.find(current);
    if (it_c != graph.end())
    {
        if (it_c->second <= path_len)
            return numeric_limits<unsigned>::max();
    }

    graph[current] = path_len;

    if (current == target) return path_len;

    unsigned ret = numeric_limits<unsigned>::max();

    for (auto b : buttons)
    {
        state new_state = current ^ b;
        ret = std::min(ret, min_presses(graph, buttons, target, new_state, path_len + 1));
    }
    
    return ret;
}

int main(int argc, char** argv)
{
    fstream f(argv[1]);

    if (!f.is_open()) return -1;

    string line;

    vector<input> inputs;

    while (getline(f, line))
    {
        istringstream s(std::move(line));

        s.ignore(1, '[');

        input line {};

        int idx = 0;

        while (s.peek() != ']')
        {
            if (s.get() == '#')
                set_bit(line.target, idx);
            ++idx;
        }

        s.ignore(1, ']');
        s.ignore(1, ' ');
        
        while (s.peek() != '{')
        {
            s.ignore(1, '(');
            string numbers;
            getline(s, numbers, ')');
            button b = 0;
            for (auto vals = get_numbers(numbers); auto n : vals)
            {
                set_bit(b, n);
            }
            line.buttons.emplace_back(b);
            s.ignore(1, ' ');        
        }

        inputs.emplace_back(line);
    }

    uint64_t sum = 0;

    for (auto& i : inputs)
    {
        cout << i << '\n';
        
        unordered_map<state, unsigned> graph;
        const auto ret = min_presses(graph, i.buttons, i.target);
        cout << ret << '\n';
        sum += ret; 
    }

    cout << "Min presses sum: " << sum << '\n';

    return 0;
}
