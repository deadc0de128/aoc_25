#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <sstream>
#include <string>
#include <algorithm>
#include <cassert>
#include <map>
#include <list>
#include <limits>

using namespace std;

struct position
{
    int64_t x;
    int64_t y;

    bool operator==(const position& o) const
    {
        return x == o.x && y == o.y;
    }
};

ostream& operator<<(ostream& o, const position& p)
{
    return o << "[" << p.x << ", " << p.y << "]";
}

auto area(const position& p1, const position& p2)
{
    const auto dx = abs(p1.x - p2.x) + 1;
    const auto dy = abs(p1.y - p2.y) + 1;

    return dx*dy;
}

struct edge
{
    position s, e;

    bool operator==(const edge& o) const
    {
        return s == o.s && e == o.e;
    }

    bool isvert() const { return s.x == e.x; }
    bool ishorz() const { return s.y == e.y; }
    
    void normalise()
    {
        if (isvert()) 
        {
            if (e.y > s.y)
                e.y--;
            else
                e.y++;

            if (e.y < s.y)
                swap(e.y, s.y);
        }
        else
        {
            assert(ishorz());
            if (e.x > s.x)
                e.x--;
            else
                e.x++;

            if (e.x < s.x)
                swap(e.x, s.x);
        }
    }
};

ostream& operator<<(ostream& o, const edge& e)
{
    return o << "{ " << e.s << ", " << e.e << " }";
}

bool intersect(const edge& e1, const edge& e2)
{
    assert(e1.s.x == e1.e.x || e1.s.y == e1.e.y);
    assert(e2.s.x == e2.e.x || e2.s.y == e2.e.y);

    if (e1.isvert()) // vert
    {
        if (e2.isvert()) // vert
        {
            return e1.s.x == e2.s.x && ((e1.s.y <= e2.s.y && e2.s.y <= e1.e.y) || (e1.s.y <= e2.e.y && e2.e.y <= e1.e.y));
        }
        else
        {
            return (e2.s.x <= e1.s.x && e1.s.x <= e2.e.x) && (e1.s.y <= e2.s.y && e2.s.y <= e1.e.y);
        }
    }
    else // horz
    {
        if (e2.ishorz()) // horz
        {
            return e1.s.y == e2.s.y && ((e1.s.x <= e2.s.x && e2.s.x <= e1.e.x) || (e1.s.x <= e2.e.x && e2.e.x <= e1.e.x));
        }
        else
        {
            return (e2.s.y <= e1.s.y && e1.s.y <= e2.e.y) && (e1.s.x <= e2.s.x && e2.s.x <= e1.e.x);
        }
    }
}

struct range
{
    int64_t s;
    int64_t e;
    bool valid;
};

ostream& operator<<(ostream& o, const range& r)
{
    return o << "[" << r.s << ", " << r.e <<  ", valid: " << r.valid << "]";
}

using raster_lines = map<int64_t, vector<range>>; // map y coord to [ x coord, edge ] pairs

raster_lines rasterise(const std::vector<edge>& edges)
{
    map<int64_t, list<const edge*>> lines;
    for (const auto& e : edges)
    {
        lines[e.s.y].emplace_back(&e);
        if (e.isvert() && e.s.y != e.e.y)
            lines[e.e.y].emplace_back(&e);
    }

    map<int64_t, const edge*> vert_edges;

    const auto y_min = lines.begin()->first;
    const auto y_max = (--lines.end())->first;
    for (int64_t y = y_min; y != y_max; ++y)
    {
        auto& line = lines[y];
        for (auto it_x = line.cbegin(); it_x != line.cend(); ++it_x) 
        {          
            if ((*it_x)->isvert())
            {
                if ((*it_x)->s.y == y)  // vertical and starts in current line
                {
                    vert_edges[(*it_x)->s.x] = *it_x;
                }
                
                if ((*it_x)->e.y == y)  // vertical and ends in current line
                {
                    vert_edges.erase((*it_x)->e.x);
                }
            }
        }

        for (auto [x, edge] : vert_edges)
            if (edge->s.y != y && edge->e.y != y)
                line.emplace_back(edge);
    }

    for (auto& p : lines)
    {
        p.second.sort([](const auto& l, const auto& r) { return l->s.x < r->s.x; });
    }

    raster_lines ret;

    for (auto& [y, line] : lines)
    {
        range r;
        r.s = (*line.cbegin())->s.x;
        r.e = (*line.cbegin())->e.x;
        r.valid = true;

        bool added_start = true;
        bool pushed = false;

        if (r.s) 
        {
            ret[y].emplace_back(0, r.s - 1, false);  // [0 .. x[0]-1 ] - invalid
        }

        for (auto it_x = ++line.cbegin(); it_x != line.cend(); ++it_x)
        {
            const auto edge = *it_x;
            
            if (edge->s.x == r.e + 1)
            {
                r.e = edge->e.x;
                pushed = false;
            }
            else
            {
                if (!added_start)
                {
                    ret[y].emplace_back(r.e + 1, edge->s.x - 1, false);
                    r.s = edge->s.x;
                    r.e = edge->e.x;
                    added_start = true;
                    pushed = false;
                }
                else
                {
                    r.e = edge->e.x;
                    ret[y].emplace_back(r);
                    pushed = true;
                }
            }
        }

        if (!pushed)
        {
            ret[y].emplace_back(r);
        }

        ret[y].emplace_back(r.e + 1, numeric_limits<int64_t>::max(), false);
    }

    return ret;    
}

bool is_inside_loop(const raster_lines& lines, position p1, position p2)
{
    const int64_t min_x = std::min(p1.x, p2.x);
    const int64_t max_x = std::max(p1.x, p2.x);
    const int64_t min_y = std::min(p1.y, p2.y);
    const int64_t max_y = std::max(p1.y, p2.y);    

    const auto it_s = lines.find(min_y);
    const auto it_e = ++lines.find(max_y);

    for (auto it = it_s; it != it_e; ++it)    
    {
        const auto& line = it->second;

        bool min_x_inside = false, max_x_inside = false;

        for (const auto& r : line)
        {
            if (r.valid)
            {
                if (r.s <= min_x && min_x <= r.e)
                    min_x_inside = true;
                if (r.s <= max_x && max_x <= r.e)
                    max_x_inside = true;
            }
            else
            {
                if (min_x <= r.s && r.s <= max_x)
                    return false;
                if (min_x <= r.e && r.e <= max_x)
                    return false;
            }
        }

        if (!min_x_inside || !max_x_inside) return false;
    }

    return true;
}

int main(int argc, char** argv)
{
    assert(!intersect({{10,7},{11,7}},{{9,6},{9,7}}));
    assert(!intersect({{9,6},{9,7}},{{10,7},{11,7}}));
    assert(intersect({{1,7},{5,7}},{{3,1},{3,10}}));
    assert(intersect({{3,1},{3,10}},{{1,7},{5,7}}));

    fstream f(argv[1]);

    if (!f.is_open()) return -1;

    vector<position> positions;

    string line;
    while (getline(f, line))
    {
        if (line.empty()) break;
        replace(line.begin(), line.end(), ',', ' ' );
        istringstream s(std::move(line));
        position b;
        s >> b.x;
        s >> b.y;
        positions.emplace_back(b);
    }

    if (positions.empty()) return -1;

    vector<edge> edges;
    for (size_t i = 0; i < positions.size()-1; ++i)
    {
        if (positions[i] == positions[i+1]) continue; // zero length edge
        edges.emplace_back(positions[i], positions[i+1]);
        edges.back().normalise();
    }
    edges.emplace_back(positions[positions.size()-1], positions[0]); // close the loop
    edges.back().normalise();

    for (size_t e1 = 0; e1 < edges.size(); ++e1)
    {
        for (size_t e2 = e1 + 1; e2 < edges.size(); ++e2)
        {
            if (intersect(edges[e1], edges[e2]))
            {
                cout << edges[e1] << " and " << edges[e2] << " intersect\n";
            }
        }
    }

    const auto lines = rasterise(edges);
 
    /*for (const auto& [y, line] : lines)
    {
        cout << y << ": ";
        for (const auto& r: line)
        {
            cout << r << " ";
        }
        cout << '\n';
    }*/

    int64_t max_area = 0;

    for (uint64_t id1 = 0; id1 < positions.size(); ++id1)
    {
        for (uint64_t id2 = id1 + 1; id2 < positions.size(); ++id2)
        {
            if (is_inside_loop(lines, positions[id1], positions[id2]))
                max_area = max(max_area, area(positions[id1], positions[id2]));
        }    
    }

    cout << "Max area: " << max_area << '\n';
    return 0;
}
