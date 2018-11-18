#include "ShuntingYard.h"

#include <iostream>
#include <chrono>
#include <fstream>

using ShuntingYardInt = ShuntingYard<int>;

bool shunting_yard_perf(const char* filename)
{
    std::ifstream f(filename);
    if (!f)
    {
        std::cerr << "File " << filename << " not found" << std::endl;
        return false;
    }

    ShuntingYardInt shunting_yard;
    ShuntingYardInt::Result rc{ ShuntingYardInt::ParseResult::Incomplete, 0 };
    std::chrono::time_point<std::chrono::system_clock> start_point{ std::chrono::system_clock::now() };

    while (rc.first == ShuntingYardInt::ParseResult::Incomplete)
    {
        char buffer[8192];
        f.read(buffer, sizeof(buffer));

        if (f)
        {
            rc = shunting_yard.parse(buffer, sizeof(buffer));
        }
        else
        {
            if (f.gcount() < 0)
            {
                break;
            }
            rc = shunting_yard.parse(buffer, static_cast<size_t>(f.gcount()));
        }
    }

    if (rc.first == ShuntingYardInt::ParseResult::Success)
    {
		std::chrono::time_point<std::chrono::system_clock> end_point{ std::chrono::system_clock::now() };
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end_point - start_point);
        std::cout << "Result of expression is: " << rc.second << ", spend time: " << milliseconds.count() << " milliseconds.";
    }
    else
    {
        std::cerr << "Could not compute an expression. " << std::endl;
    }

    return rc.first == ShuntingYardInt::ParseResult::Success;
}

int main(int argc, const char *argv[])
{
    if (argc == 1)
    {
        std::cout << "Check performance of Shunting Yard algorithm. " << std::endl <<
            "Please provide filename with expression." << std::endl;
        return 1;
    }

    return shunting_yard_perf(argv[1]) ? 0 : 1;
}
