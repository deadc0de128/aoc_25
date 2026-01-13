
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <sstream>
#include <string>
#include <algorithm>
#include <cassert>

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
    
    void normalise()
    {
        if (s.x == e.x) 
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
            assert(s.y == e.y);
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

    if (e1.s.x == e1.e.x) // vert
    {
        if (e2.s.x == e2.e.x) // vert
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
        if (e2.s.y == e2.e.y) // horz
        {
            return e1.s.y == e2.s.y && ((e1.s.x <= e2.s.x && e2.s.x <= e1.e.x) || (e1.s.x <= e2.e.x && e2.e.x <= e1.e.x));
        }
        else
        {
            return (e2.s.y <= e1.s.y && e1.s.y <= e2.e.y) && (e1.s.x <= e2.s.x && e2.s.x <= e1.e.x);
        }
    }
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


    int64_t max_area = 0;

    for (uint64_t id1 = 0; id1 < positions.size(); ++id1)
    {
        for (uint64_t id2 = id1 + 1; id2 < positions.size(); ++id2)
        {
            max_area = max(max_area, area(positions[id1], positions[id2]));
        }    
    }

    cout << "Max area: " << max_area << '\n';
    return 0;
}
