#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <algorithm>

using namespace std;

struct box
{
    uint64_t id;
    uint64_t x;
    uint64_t y;
    uint64_t z;
    bool visited;
};

ostream& operator<<(ostream& o, const box& b)
{
    return o << "[" << b.x << ", " << b.y << ", " << b.z << "]";
}

uint64_t distance(const box& b1, const box& b2)
{
    const int64_t dx = (int64_t)b1.x - (int64_t)b2.x;
    const int64_t dy = (int64_t)b1.y - (int64_t)b2.y;
    const int64_t dz = (int64_t)b1.z - (int64_t)b2.z;

    return dx*dx + dy*dy + dz*dz;
}

struct box_dist
{
    box *b1, *b2;
    uint64_t d;
    bool operator<(const box_dist& o) const
    {
        return d < o.d;
    }
    bool operator>(const box_dist& o) const
    {
        return d > o.d;
    }
};

int main(int argc, char** argv)
{
    fstream f(argv[1]);

    const int max_conn = std::stoi(argv[2]);

    if (!f.is_open()) return -1;

    multimap<uint64_t, uint64_t> links;
    priority_queue<box_dist, deque<box_dist>, greater<box_dist>> distances;
    vector<box> boxes;

    uint64_t next_id = 0;

    string line;
    while (getline(f, line))
    {
        if (line.empty()) break;
        replace(line.begin(), line.end(), ',', ' ' );
        istringstream s(std::move(line));
        box b;
        s >> b.x;
        s >> b.y;
        s >> b.z;
        b.id = next_id++;
        b.visited = false;
        boxes.emplace_back(b);
    }

    for (uint64_t id1 = 0; id1 < next_id; ++id1)
    {
        for (uint64_t id2 = id1 + 1; id2 < next_id; ++id2)
        {
            distances.emplace(&boxes[id1], &boxes[id2], distance(boxes[id1], boxes[id2]));
        }    
    }

    int conn = 0;
    while (!distances.empty() && conn < max_conn)
    {
        const auto& d = distances.top();
        cout << *d.b1 << " -- " << *d.b2 << " == " << d.d << '\n';
        links.emplace(d.b1->id, d.b2->id); // do not check if belong to the same cicuit
        links.emplace(d.b2->id, d.b1->id); // do not check if belong to the same cicuit
        distances.pop();
        ++conn;
    }

    priority_queue<uint64_t> sizes;

    queue<box*> subgraph;

    for (auto& b : boxes)
    {
        if (b.visited) continue;
        uint64_t size = 0;

        cout << "-----------------------------\n";

        subgraph.push(&b);

        while (!subgraph.empty())
        {
            box* b = subgraph.front();
            if (!b->visited)
            {
                b->visited = true;
                cout << *b << '\n';
                size++;

                auto [it, it_e] = links.equal_range(b->id);
            
                for (; it != it_e; ++it)
                {
                    box* b = &boxes[it->second];
                    if (!b->visited)
                        subgraph.push(b);
                }
            }

            subgraph.pop();
        }
        
        cout << "Subgraph size: " << size << '\n';
        cout << "-----------------------------\n";
        sizes.push(size);
    }

    uint64_t result = 1;

    for (int i = 0; i < 3; ++i)
    {
        if (sizes.empty()) return -1;
        result *= sizes.top();
        sizes.pop();
    }

    cout << "Result: " << result << '\n';

    return 0;
}
