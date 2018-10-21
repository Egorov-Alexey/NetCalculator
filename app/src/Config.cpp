#include "Config.h"

#include <iostream>
#include <thread>

#include <boost/program_options.hpp>

boost::optional<Config> get_config(int argc, const char *argv[])
{
    Config default_config{0, 0, std::thread::hardware_concurrency()};

    namespace po = boost::program_options;
    po::options_description desc("General options");
    desc.add_options()
        ("help,h", "Show help")
        ("port,p",    po::value<decltype(default_config.port)>(&default_config.port),       "Listen port")
        ("clients,c", po::value<decltype(default_config.clients)>(&default_config.clients), "Maximum number of simultaneous clients")
        ("threads,t", po::value<decltype(default_config.threads)>(&default_config.threads), "Number of threads (default value is std::thread::hardware_concurrency())");

    po::variables_map vm;
    boost::optional<Config> result;
    bool incomplete = false;

    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    }
    catch (...)
    {
        std::cout << "Invalid parameters." << std::endl;
        std::cout << desc << std::endl;
        return result;
    }

    if (argc < 2 || vm.count("help"))
    {
        std::cout << desc << std::endl;
        return result;
    }

    if (!vm.count("port"))
    {
        std::cout << "Parameter 'port' is not defined." << std::endl;
        incomplete = true;
    }
    else if (default_config.port < 1024)
    {
        std::cout << "Parameter 'port' must be >= 1024" << std::endl;
        incomplete = true;
    }

    if (!vm.count("clients"))
    {
        std::cout << "Parameter 'clients' is not defined." << std::endl;
        incomplete = true;
    }
    else if (!default_config.clients)
    {
        std::cout << "Parameter 'clients' must be positive." << std::endl;
        incomplete = true;
    }

    if (!default_config.threads)
    {
        std::cout << "Parameter 'threads' must be positive." << std::endl;
        incomplete = true;
    }

    if (default_config.clients < default_config.threads)
    {
        std::cout << "Number of threads(" << default_config.threads
            << ") can't exceed number of clients(" << default_config.clients << ")." << std::endl;
        return result;
    }

    result = default_config;

    return result;
}
