#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <functional>

using namespace std;

struct position
{
    int x;
    int y;
    int64_t timelines;
};

struct ray
{
    int sx;
    int sy;
    position* source;
    bool operator<(const ray& o) const 
    {
        if (sy == o.sy) return sx < o.sx;
        return sy < o.sy;
    }
    bool operator>(const ray& o) const
    {
        if (sy == o.sy) return sx > o.sx;
        return sy > o.sy;
    }
};

ostream& operator<<(ostream& s, position p)
{
    return s << '(' << p.x << ", " << p.y << ')';
}

int main(int argc, char** argv)
{
    ifstream f(argv[1]);

    if (!f.is_open()) return -1;

    position start;

    std::vector<position> splitters;

    int y = 0;
    int x_size = 0;

    string line;
    while (getline(f, line))
    {
        if (line.empty()) return -1;

        if (y == 0)
        {
            auto pos = line.find('S');
            if (pos == string::npos) return -1;
            x_size = line.size();
            start = position { (int)pos, 0, 1 };
        }

        auto pos = line.find('^', 0);
        while (pos != string::npos)
        {
            splitters.emplace_back(pos, y, 0);
            pos = line.find('^', pos + 1);
        }

        y++;
    }

    cout << start << '\n';
    for (auto p : splitters)
        cout << p << "; ";
    cout << '\n';

    vector<vector<int>> splt_y_pos; // x -> y pos
    splt_y_pos.resize(x_size);
    vector<vector<vector<position>::iterator>> splt_it; // x -> y pos
    splt_it.resize(x_size);
    
    for (auto it = splitters.begin(); it != splitters.end(); ++it)
    {
        splt_y_pos[it->x].push_back(it->y);
        splt_it[it->x].push_back(it);
    }
    
    priority_queue<ray, std::deque<ray>, std::greater<ray>> rays;
    rays.push(ray { start.x, start.y + 1, &start } ); // initial ray is just below start point

    int64_t timelines = 0;

    while (!rays.empty())
    {
        auto r = rays.top();
        rays.pop();
        const auto& positions = splt_y_pos[r.sx];
        const auto& iters = splt_it[r.sx];

        auto it = upper_bound(positions.begin(), positions.end(), r.sy);
        if (it != positions.end())
        {
            position* pos = &*iters[distance(positions.begin(), it)];
            if (!pos->timelines)
            {
                ray left{ r.sx - 1, *it, pos };
                ray right{ r.sx + 1, *it, pos };
                rays.push(left);
                rays.push(right);
            }
            pos->timelines += r.source->timelines;
        }
        else
        {
            cout << r.source->timelines << '\n';
            timelines += r.source->timelines;
        }
    }
// 1. need to track end of rays  not origins
// 2. rays should be processed from low y coords to high !
    cout << "Timelines: " << timelines << '\n';   

    return 0;
}
