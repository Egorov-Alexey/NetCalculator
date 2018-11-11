#include <Config.h>
#include <iostream>
#include <vector>

namespace
{
bool operator==(const Config& lhs, const Config& rhs)
{
    return lhs.port == rhs.port && lhs.clients == rhs.clients && lhs.threads == rhs.threads;
}

struct TestData
{
    int result;
    Config config;
    std::vector<const char*> args;
};

bool test_config(const TestData& test_data)
{
    boost::optional<Config> config = get_config(test_data.args.size(), test_data.args.data());

    if (!config)
    {
        return !test_data.result;
    }

    return test_data.config == config.get();
}

const TestData TestCases[] =
{
    {false, Config{}, {"dummy"}},
    {false, Config{}, {"dummy", "-h"}},
    {false, Config{}, {"dummy", "-p", "1024"}},
    {false, Config{}, {"dummy", "-c", "10"}},
    {false, Config{}, {"dummy", "-t", "2"}},
    {false, Config{}, {"dummy", "-p", "1024", "-t", "2"}},
    {false, Config{}, {"dummy", "-c", "10", "-t", "2"}},
    {false, Config{}, {"dummy", "-p", "x", "-c", "10", "-t",  "2"}},
    {false, Config{}, {"dummy", "-p", "1024", "-c", "x", "-t",  "2"}},
    {false, Config{}, {"dummy", "-p", "1024", "-c", "10", "-t",  "x"}},
    {false, Config{}, {"dummy", "-p", "0", "-c", "10", "-t",  "2"}},
    {false, Config{}, {"dummy", "-p", "1023", "-c", "10", "-t",  "2"}},
    {false, Config{}, {"dummy", "-p", "1024", "-c", "0", "-t",  "2"}},
    {false, Config{}, {"dummy", "-p", "1024", "-c", "10", "-t",  "0"}},
    {false, Config{}, {"dummy", "-p", "1024", "-c", "2", "-t",  "10"}},
    {true,  Config{1024, 10, 2}, {"dummy", "-p", "1024", "-c", "10", "-t",  "2"}}
};
}

int main()
{
    for (const TestData& test_case : TestCases)
    {
        if (!test_config(test_case))
        {
            return 1;
        }
    }

    return 0;
}
