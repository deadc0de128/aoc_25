#include <iostream>
#include <fstream>
#include <vector>
#include <optional>
#include <cstdint>
#include <charconv>

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

int main(int argc, char** argv)
{
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

    int count = 0;

    for (const auto id : ids)
    {
        for (const auto& r : ranges)
        {
            if (r.inside(id))
            {
                count++;
                break;
            }
        }            
    }

    cout << "Fresh: " << count << '\n';

    return 0;
}
