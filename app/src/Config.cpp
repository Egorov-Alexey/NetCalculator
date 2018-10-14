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
        ("port,p",    po::value<unsigned int>(&default_config.port),    "Listen port")
        ("clients,c", po::value<unsigned int>(&default_config.clients), "Maximum number of simultaneous clients")
        ("threads,t", po::value<unsigned int>(&default_config.threads), "Number of threads (default value is std::thread::hardware_concurrency())");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    boost::optional<Config> result;

    bool not_defined = false;
    if (!vm.count("port"))
    {
        std::cout << "Parameter 'port' is not defined." << std::endl;
        not_defined = true;
    }

    if (!vm.count("clients"))
    {
        std::cout << "Parameter 'clients' is not defined." << std::endl;
        not_defined = true;
    }

    if (not_defined || argc < 2 || vm.count("help"))
    {
        std::cout << desc << std::endl;
        return result;
    }

    if (default_config.clients < default_config.threads)
    {
        std::cout << "Number of threads can't exceed number of clients." << std::endl;
        return result;
    }

    result = default_config;

    return result;
}
