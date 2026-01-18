#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <cmath>
#include <tuple>

using namespace std;

using state = uint32_t;
using button = uint32_t;
using joltage_vector = vector<int>;
using matrix = vector<vector<double>>;

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
    joltage_vector joltage;
};

ostream& operator<<(ostream& s, const input& in)
{
    s << '[';
    for (int i = 0; i < 10; ++i)
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
    s << "{";
    for (auto j : in.joltage)
    {
        s << (int)j << ", ";
    }
    s << "}";

    return s;
}

void print_matrix(vector<vector<double>> m)
{
    for (int row = 0; row < m.size(); ++row)
    {
        for (int col = 0; col < m[row].size(); ++col)
        {
            cout << setprecision(2) << setw(8) << fixed << m[row][col];
            if (col == m[row].size() - 2) cout << " |";
        }
        cout << '\n';
    }
}

bool non_zero(const double& n)
{ 
    return fabs(n) >= 0.000001;
};

// solve system of linear equations
// using gaussian elimination
// return results if prev_results are empty or
// sum of new results is less than prev_results
bool solve(matrix eq, vector<double>& prev_results)
{
    if (eq.empty()) return false;

    vector<double> results;
    results.resize(eq[0].size() - 1);

    for (int i = 0; i < eq[0].size() - 1; ++i)
    {
        int idx = 0;
        double val = 0;
        matrix::iterator it_s = eq.end();

        // find simple equalities k*x = a
        for (auto it = eq.begin(); it != eq.end(); ++it)
        {
            if (count_if(it->begin(), it->end() - 1, non_zero) != 1) continue;
            it_s = it;
            idx = distance(it->begin(), find_if(it->begin(), it->end() - 1, non_zero));
            val = (*it)[it->size()-1] / (*it)[idx];
            break;
        }

        if (it_s == eq.end()) // cannot find simple equality
        {
            return false;
        }
        if (signbit(val) && non_zero(val)) // negative
        {
            return false;
        }
        double _;
        double fract = modf(val, &_);
        if (non_zero(fract) && non_zero(1.0 - fract)) // non integer
        {
            return false;
        }

        results[idx] = val;

        // substitute found variable into other equations
        for (auto it = eq.begin(); it != eq.end(); ++it)
        {
            if (non_zero((*it)[idx]))
            {
                (*it)[it->size()-1] -= (*it)[idx]*val;
                (*it)[idx] = 0;
            }
        }
    }

    // check if new result is better
    if (prev_results.empty() || 
        accumulate(prev_results.begin(), prev_results.end(), 0.0) > accumulate(results.begin(), results.end(), 0.0))
    {
        prev_results = results;
        return true;
    }

    return false;
}

static const int MAX_JOLTAGE = 200;

// sequence of integer tuples of size 3, each component is in [0 .. MAX_JOLTAGE] range,
// ordered by the sum of components from smallest to biggest
static auto sum3 = []()
{
    auto sum3_comp = [](const tuple<int,int,int>& l, const tuple<int,int,int>& r)
                     {
                        const int suml = get<0>(l) + get<1>(l) + get<2>(l);
                        const int sumr = get<0>(r) + get<1>(r) + get<2>(r);
                        return suml < sumr;
                     };

    vector<tuple<int,int,int>> sum3;
    sum3.reserve(MAX_JOLTAGE*MAX_JOLTAGE*MAX_JOLTAGE);

    for (int i = 0; i < MAX_JOLTAGE; ++i)
        for (int j = 0; j < MAX_JOLTAGE; ++j)
          for (int k = 0; k < MAX_JOLTAGE; ++k)
              sum3.emplace_back(i, j, k);

    sort(sum3.begin(), sum3.end(), sum3_comp);

    return sum3;
}();

// sequence of integer tuples of size 2, each component is in [0 .. MAX_JOLTAGE] range,
// ordered by the sum of components from smallest to biggest
static auto sum2 = []()
{
    auto sum2_comp = [](const tuple<int,int>& l, const tuple<int,int>& r)
                     {
                        const int suml = get<0>(l) + get<1>(l);
                        const int sumr = get<0>(r) + get<1>(r);
                        return suml < sumr;
                     };

    vector<tuple<int,int>> sum2;
    sum2.reserve(MAX_JOLTAGE*MAX_JOLTAGE);

    for (int i = 0; i < MAX_JOLTAGE; ++i)
        for (int j = 0; j < MAX_JOLTAGE; ++j)
              sum2.emplace_back(i, j);

    sort(sum2.begin(), sum2.end(), sum2_comp);

    return sum2;
}();

optional<unsigned> min_presses_joltage(const vector<button>& buttons,
                                       const joltage_vector& target_joltage)
{
    // create system of linear equations

    matrix eq(target_joltage.size());
    for (int i = 0; i < target_joltage.size(); ++i)
    {
        eq[i].resize(buttons.size() + 1);
        eq[i][buttons.size()] = target_joltage[i];
    }

    for (int i = 0; i < buttons.size(); ++i)
    {
        for (int j = 0; j < target_joltage.size(); ++j)
        {
            if (get_bit(buttons[i], j))
            {
                eq[j][i] = 1;
            }
        }
    }

    auto compare_rows = [](const auto& l, const auto& r) { 
        for (int i = 0; i < l.size() - 1; ++i)
        {
            if (fabs(l[i]) > fabs(r[i]))
                return true;
            else if (fabs(l[i]) < fabs(r[i]))
                return false;
        }

        return false; 
    }; 

    sort(eq.begin(), eq.end(), compare_rows);

    cout << "-------- Original ----------\n";
    print_matrix(eq);

    // transform matrxi to row echelon from

    for (int row1 = 0; row1 < eq.size() - 1; ++row1)
    {
        int first_non_zero = distance(eq[row1].begin(), find_if(eq[row1].begin(), eq[row1].end(), [](auto v) { return non_zero(v); }));
        if (first_non_zero == eq[row1].size()) continue;
        for (int row2 = row1 + 1; row2 < eq.size(); ++row2)        
        {
            const auto k = eq[row2][first_non_zero]/eq[row1][first_non_zero];
            for (int i = first_non_zero; i < eq[0].size(); ++i)
            {
                eq[row2][i] -= k * eq[row1][i];
            }
        }

        sort(eq.begin()+row1, eq.end(), compare_rows);
        
    }

    cout << "-------- Row echelon -------\n";
    print_matrix(eq);

    // find "missing" rows preventing existance of single solution
                        
    vector<int> missing_indices;    
                        
    int col = 0, row = 0;
    for (; row < eq.size();)
    {
        if (col >= buttons.size()) break;

        if (!non_zero(eq[row][col])) 
        {
            missing_indices.push_back(col);
            col++;
        }
        else
        {
            col++;
            row++;
        }
    }

    for (int i = col; i < (int)buttons.size(); ++i)
    {
        missing_indices.push_back(i);
    }
 
    if (!missing_indices.empty())
    {
        cout << "Missing: ";
        for (auto idx : missing_indices) cout << idx << " ";
        cout << '\n';
    }

    // solve by bruteforcing, "missing" rows will be replaced by simple equalities

    bool solved = false;

    vector<double> results;

    auto last_eq_compl = eq;

    if (missing_indices.size() == 0)
    {
        if (solve(eq, results))
        {
            solved = true;
        }
    }
    else if (missing_indices.size() == 1)
    {
        for (int i = 0; i < MAX_JOLTAGE; ++i)
        {
            auto eq_compl = eq;
            vector<double> row(buttons.size() + 1);
            row[missing_indices[0]] = 1;
            row[row.size() - 1] = i;
            eq_compl.insert(next(eq_compl.begin(), missing_indices[0]), row);
            if (solve(eq_compl, results))
            {
                last_eq_compl = eq_compl;
                solved = true;
            }
        }
    }
    else if (missing_indices.size() == 2)
    {
        for (const auto& t : sum2)
        {
            auto eq_compl = eq;
            {
                vector<double> row(buttons.size() + 1);
                row[missing_indices[0]] = 1;
                row[row.size() - 1] = get<0>(t);
                eq_compl.insert(next(eq_compl.begin(), missing_indices[0]), row);
            }
            {
                vector<double> row(buttons.size() + 1);
                row[missing_indices[1]] = 1;
                row[row.size() - 1] = get<1>(t);
                eq_compl.insert(next(eq_compl.begin(), missing_indices[1]), row);
            }
            if (solve(eq_compl, results))
            {
                last_eq_compl = eq_compl;
                solved = true;
            }
        }
    }
    else if (missing_indices.size() == 3)
    {
        for (const auto& t : sum3)
        {
            auto eq_compl = eq;
            {
                vector<double> row(buttons.size() + 1);
                row[missing_indices[0]] = 1;
                row[row.size() - 1] = get<0>(t);
                eq_compl.insert(next(eq_compl.begin(), missing_indices[0]), row);
            }
            {
                vector<double> row(buttons.size() + 1);
                row[missing_indices[1]] = 1;
                row[row.size() - 1] = get<1>(t);
                eq_compl.insert(next(eq_compl.begin(), missing_indices[1]), row);
            }
            {
                vector<double> row(buttons.size() + 1);
                row[missing_indices[2]] = 1;
                row[row.size() - 1] = get<2>(t);
                eq_compl.insert(next(eq_compl.begin(), missing_indices[2]), row);
            }
            if (solve(eq_compl, results))
            {
                last_eq_compl = eq_compl;
                solved = true;
            }
        }
    }
    else
    {
        solved = false;
    }

    // check results

    if (solved)
    {
        cout << "----------------------------\n";
        cout << "Results: [ ";
        for (auto r : results)
            cout << r << " ";
        cout << "]\n";

        vector<int> joltage(target_joltage.size());
        for (int i = 0; i < buttons.size(); ++i)
        {
            for (int k = 0; k < joltage.size(); ++k)
                joltage[k] += get_bit(buttons[i], k) * (int)(results[i]+0.5);
        }

        if (joltage != target_joltage)
        {
            cout << "Wrong: ";
            for (auto j : joltage) cout << j << ", ";
            cout << '\n';

            cout << "==================================================\n";
            print_matrix(last_eq_compl);
            cout << "==================================================\n";
        }
        else
        {
            return (int)(accumulate(results.begin(), results.end(), 0.0) + 0.5);
        }
    }
        
    return {};
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

        s.ignore(1, '{');
        while (s.peek() != '}')
        {
            int32_t j;
            s >> j;
            line.joltage.emplace_back(j);
            if (s.peek() == ',')
                s.ignore(1, ',');
        }

        inputs.emplace_back(line);
    }

    uint64_t sum = 0;

    for (auto& i : inputs)
    {
        cout << "-------------------------------------------------------\n";
        cout << i << '\n';
        
        const auto ret = min_presses_joltage(i.buttons, i.joltage);
        if (ret)
        {
            cout << "Min presses: " << *ret << '\n';
            sum += *ret; 
        }
        else
        {
            cout << "Does not compute\n";
            return -1;
        }
    }

    cout << "Min presses sum: " << sum << '\n';

    return 0;
}

/*
joltage_vector push_button(const joltage_vector& v, button b)
{
    joltage_vector ret {v};

    for (size_t i = 0; i < v.size(); ++i)
    {
        ret[i] += get_bit(b, i);
    }

    return ret;
}

bool valid_joltage(const joltage_vector& j, const joltage_vector& target)
{
    assert(j.size() == target.size());

    for (size_t i = 0; i < j.size(); ++i)
        if (j[i] > target[i]) return false;

    return true;
}

template <>
struct hash<joltage_vector>
{
    size_t operator()(const joltage_vector& v) const noexcept
    {
        size_t hash = 0;
        for (auto i : v)
        {
            // boost hash_combine
            hash ^= std::hash<joltage_vector::value_type>{}(i) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

using jolt_graph = unordered_map<joltage_vector, unsigned>;

unsigned min_presses_joltage_brutforce(const vector<button>& buttons,
                                       const joltage_vector& target_joltage)
{
    queue<tuple<joltage_vector, button, unsigned>> q;

    jolt_graph graph;
    graph.reserve(25 * 1024 * 1024);

    // initial state - all zeroes
    for (auto b : buttons)
    {
        q.emplace(joltage_vector(target_joltage.size()), b, 0);
    }

    while (!q.empty())
    {
        auto& [joltage, b, depth] = q.front();

        if (joltage == target_joltage)
        {
            size_t max_bucket_count = 0;
            for (size_t i = 0; i < graph.bucket_count(); ++i)
                max_bucket_count = max(max_bucket_count, graph.bucket_size(i));
            cout << graph.size() << " in " << graph.bucket_count() << " max = " << max_bucket_count << '\n';
            return depth;
        }

        const auto new_joltage = push_button(joltage, b);
        q.pop();

        if (!valid_joltage(new_joltage, target_joltage)) continue;

        auto it_g = graph.find(new_joltage);
        if (it_g != graph.end() && it_g->second <= depth + 1) 
            continue;

        graph[new_joltage] = depth + 1;

        for (auto b : buttons)
            q.emplace(new_joltage, b, depth + 1);
    }

    return numeric_limits<unsigned>::max();
}
*/


