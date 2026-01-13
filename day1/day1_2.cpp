#include <iostream>
#include <string>

int main()
{
    int pos = 50;
    unsigned zero = 0;

    for (std::string line; std::getline(std::cin, line);)
    {
        std::cout << line << '\n';

        if (line.length() < 2) return -1;
        const char dir = line[0];
        const int num = std::stoi(line.substr(1));

        if (dir == 'R')      
        {     
            zero += (pos + num) / 100;
            if ((pos+num) % 100 == 0) zero--;
            pos = (pos + num) % 100;
            zero += (pos == 0);
        }
        else if (dir == 'L')
        {
            auto diff = abs(pos - num) % 100;
            if (diff != 0 && pos < num && pos) zero++;
            zero += abs(pos - num) / 100;
            pos = (diff != 0 && pos < num) ? 100 - diff : diff;
            zero += (pos == 0);
        }
        else
            return -1;

        std::cout << pos << " (" << zero << ")\n";
    }

    std::cout << zero << std::endl;

    return 0;
}

