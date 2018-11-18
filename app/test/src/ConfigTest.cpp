/**
 * This file contains unit-tests for get_config() function.
 * main function returns 0 if all test passed.
 * main function returns 1 if one of test failed.
 */

#include <Config.h>

#include <iostream>
#include <vector>
#include <thread>

bool operator==(const Config& lhs, const Config& rhs)
{
    return lhs.address == rhs.address &&
        lhs.port == rhs.port &&
        lhs.clients == rhs.clients &&
        lhs.threads == rhs.threads;
}

struct TestData
{
    int result;
    Config config;
    std::vector<const char*> args;
};

std::ostream& operator<<(std::ostream& s, const TestData& test_case)
{
    for (auto& arg: test_case.args)
    {
        s << arg << " ";
    }

    return s;
}

bool test_config(const TestData& test_data)
{
    boost::optional<Config> config = get_config(test_data.args.size(), test_data.args.data());

    if (!config)
    {
        return !test_data.result;
    }

    return test_data.config == config.get();
}

unsigned int get_hwc()
{
    static unsigned int hwc = std::thread::hardware_concurrency();
    return hwc ? hwc : 1;
}

const TestData test_cases[] =
{
    {false, Config{}, {"dummy"}},
    {false, Config{}, {"dummy", "-h"}},

    {false, Config{}, {"dummy", "-a", "0.0.0.0"}},
    {false, Config{}, {"dummy", "-p", "1024"}},
    {false, Config{}, {"dummy", "-c", "10"}},
    {false, Config{}, {"dummy", "-t", "2"}},

    {false, Config{}, {"dummy", "-a", "0.0.0.0", "-p", "1024"}},
    {false, Config{}, {"dummy", "-a", "0.0.0.0", "-c", "10"}},
    {false, Config{}, {"dummy", "-a", "0.0.0.0", "-t", "2"}},
    {true,  Config{"127.0.0.1", 1024, 10, get_hwc()},
        {"dummy", "-p", "1024", "-c", "10"}},
    {false, Config{}, {"dummy", "-p", "1024", "-t", "2"}},
    {false, Config{}, {"dummy", "-c", "10", "-t", "2"}},

    {true,  Config{"127.0.0.1", 1024, 10, 2},
        {"dummy", "-p", "1024", "-c", "10", "-t",  "2"}},
    {false, Config{}, {"dummy", "-a", "0.0.0.0", "-c", "10", "-t",  "2"}},
    {false, Config{}, {"dummy", "-a", "0.0.0.0", "-p", "1024", "-t",  "2"}},
    {true,  Config{"127.0.0.1", 1024, 10, get_hwc()},
        {"dummy", "-a", "127.0.0.1", "-p", "1024", "-c", "10"}},
    {false, Config{}, {"dummy", "-a", "x.0.0.0", "-p", "1024", "-c", "10", "-t", "2"}},
    {false, Config{}, {"dummy", "-a", "0.0.0.0", "-p", "x", "-c", "10", "-t", "2"}},
    {false, Config{}, {"dummy", "-a", "0.0.0.0", "-p", "1024", "-c", "x", "-t", "2"}},
    {false, Config{}, {"dummy", "-a", "0.0.0.0", "-p", "1024", "-c", "10", "-t", "x"}},

    {false, Config{}, {"dummy", "-a", "0.0.0.0", "-p", "0", "-c", "10", "-t",  "2"}},
    {false, Config{}, {"dummy", "-a", "0.0.0.0", "-p", "1023", "-c", "10", "-t",  "2"}},
    {false, Config{}, {"dummy", "-a", "0.0.0.0", "-p", "1024", "-c", "0", "-t",  "2"}},
    {false, Config{}, {"dummy", "-a", "0.0.0.0", "-p", "1024", "-c", "10", "-t",  "0"}},
    {false, Config{}, {"dummy", "-a", "0.0.0.0", "-p", "1024", "-c", "2", "-t",  "10"}},

    {true,  Config{"12.34.56.78", 1024, 10, 2},
        {"dummy", "-a", "12.34.56.78", "-p", "1024", "-c", "10", "-t",  "2"}}
};

int main()
{
    for (const TestData& test_case : test_cases)
    {
        if (!test_config(test_case))
        {
            test_config(test_case);
            std::cerr << "Test: " << test_case << "failed" << std::endl;
            return 1;
        }
    }

    return 0;
}
