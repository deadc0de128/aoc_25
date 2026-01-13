#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <optional>
#include <cstdint>
#include <charconv>
#include <concepts>
#include <cassert>
#include <cstdlib>

using namespace std;

struct range
{
    uint64_t s;
    uint64_t e;

    bool inside(uint64_t id) const { return s <= id && id <= e; }
};

ostream& operator<<(ostream& s, range r)
{
    s << r.s << " - " << r.e;
    return s;
}

std::optional<range> get_range(const std::string& line)
{
    auto pos = line.find('-');
    if (pos == string::npos)
    {
        cerr << " - not found\n";
        return {};
    }
    const char* start = line.c_str();
    const char* end = line.c_str() + pos;
    uint64_t r_start, r_end;
    auto ret = from_chars(start, end, r_start);
    if (ret.ec != errc())
    {
        cerr << "Cannot convert \"" << string_view(start, end) << "\": "
             << make_error_code(ret.ec).message() << '\n';
        return {};
    }
    start = end + 1;
    end = line.c_str() + line.size();
    ret = from_chars(start, end, r_end);
    if (ret.ec != errc())
    {
        cerr << "Cannot convert \"" << string_view(start, end) << "\": "
             << make_error_code(ret.ec).message() << '\n';
        return {};
    }
    return range { r_start, r_end };
}

struct range_pt
{
    uint64_t pt;
    enum Type { start, end };
    Type type;

    bool operator<(range_pt o) const
    {
        if (pt == o.pt)
            return type == start && o.type == end;

        return pt < o.pt;
    }
};

ostream& operator<<(ostream& o, range_pt pt)
{
    if (pt.type == range_pt::end)
        o << ", " << pt.pt << "] ";
    else
        o << "[" << pt.pt;
    return o;
}

template <forward_iterator It> requires same_as<typename It::value_type, range>
std::set<range_pt> normalise_ranges(It begin, It end)
{
    std::set<range_pt> ret;

    for (auto r = begin; r != end; ++r)
    {
        const range_pt s { r->s, range_pt::start };
        const range_pt e { r->e, range_pt::end };

        assert(s.pt <= e.pt);

        auto it = ret.lower_bound(s);

        if (it == ret.end())
        { // all pt are less or empty
            ret.emplace(s);
            ret.emplace(e);
        }
        else if (it->pt == s.pt)
        { // range with the same start or end pt

            if (it->type == range_pt::end)
            { // range with end == current range start
                it = ret.erase(it);
            }
            else
            {
                ++it;
            }

            while (it != ret.end() && it->pt <= e.pt)
            {
                it = ret.erase(it);
            }

            if (it == ret.end() || it->type == range_pt::start)
            {
                ret.emplace(e);
            }
        }
        else // it->pt > s.pt
        {
            if (it->type == range_pt::start)
            {
                it = ret.emplace(s).first;
                ++it;
            }

            while (it != ret.end() && it->pt <= e.pt)
            {
                it = ret.erase(it);
            }

            if (it == ret.end() || it->type == range_pt::start)
            {
                ret.emplace(e);
            }
        }
    }

    return ret;
}

void test1()
{
    std::vector<range> t1 { { 0, 100 }, { 0, 200 } };
    for (const auto& r : t1)
        cout << r << ", ";
    cout << '\n';
    auto n = normalise_ranges(t1.begin(), t1.end());
    for (const auto& pt : n)
        cout << pt;
    cout << '\n';
}

int main(int argc, char** argv)
{
    if (getenv("TEST"))
    {
        test1();
        return 0;
    }

    fstream f(argv[1]);

    if (!f.is_open()) return -1;

    vector<range> ranges;
    vector<uint64_t> ids;

    string line;
    while (getline(f, line))
    {
        if (line.empty()) break;
        auto r = get_range(line);
        if (!r) return -1;
        ranges.emplace_back(*r);
    }

    while (getline(f, line))
    {
        uint64_t id;
        auto ret = from_chars(line.c_str(), line.c_str() + line.size(), id);
        if (ret.ec != errc())
        {
            cerr << "Cannot convert to id \"" << line << "\": "
                 << make_error_code(ret.ec).message() << '\n';
            return -1;
        }
        ids.emplace_back(id);
    }

    for (const auto& r : ranges)
        cout << r << '\n';
    for (const auto id : ids)
        cout << id << '\n';

    uint64_t count = 0;

    /*for (const auto id : ids)
    {
        for (const auto& r : ranges)
        {
            if (r.inside(id))
            {
                count++;
                break;
            }
        }            
    }*/

    auto norm_ranges = normalise_ranges(ranges.begin(), ranges.end());

    range_pt s;

    for (const auto& pt : norm_ranges)
    {
        if (pt.type == range_pt::start)
            s = pt;
        else
            count += pt.pt - s.pt + 1;

        cout << pt;
    }

    cout << "\nFresh: " << count << '\n';

    return 0;
}
