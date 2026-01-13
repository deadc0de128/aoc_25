#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>

using namespace std;

struct position
{
    int x;
    int y;
    bool found;
};

struct ray
{
    int sx;
    int sy;
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
            start = position { (int)pos, 0, true };
        }

        auto pos = line.find('^', 0);
        while (pos != string::npos)
        {
    splitters.emplace_back(pos, y, false);
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

    queue<ray> rays;
    rays.push(ray { start.x, start.y + 1 } ); // initial ray is just below start point

    while (!rays.empty())
    {
        auto r = rays.front();
        rays.pop();
        const auto& positions = splt_y_pos[r.sx];
        const auto& iters = splt_it[r.sx];

        auto it = upper_bound(positions.begin(), positions.end(), r.sy);
        if (it != positions.end())
        {
            if (!iters[distance(positions.begin(), it)]->found)
            {
                iters[distance(positions.begin(), it)]->found = true;
                rays.push(ray { r.sx - 1, *it });
                rays.push(ray { r.sx + 1, *it });
            }
        }
    }

    cout << "Split count: " << count_if(splitters.begin(), splitters.end(), [](auto& s) { return s.found; }) << '\n';

    return 0;
}
