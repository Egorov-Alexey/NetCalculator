#include "Config.h"

#include <functional>
#include <iostream>
#include <thread>

#include <boost/program_options.hpp>

namespace
{
using Port = decltype(Config::port);
using Clients = decltype(Config::clients);
using Threads = decltype(Config::threads);
namespace po = boost::program_options;

po::options_description make_description(Config& default_config)
{
    po::options_description desc("General options");
    desc.add_options()
        ("help,h", "Show help")
        ("port,p",    po::value<decltype(default_config.port)>(&default_config.port),       "Listen port")
        ("clients,c", po::value<decltype(default_config.clients)>(&default_config.clients), "Maximum number of simultaneous clients")
        ("threads,t", po::value<decltype(default_config.threads)>(&default_config.threads), "Number of threads (default value is std::thread::hardware_concurrency())");

    return desc;
}

boost::optional<po::variables_map> parse_arguments(int argc, const char* const* argv, const po::options_description& desc)
{
    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        return vm;
    }
    catch (...)
    {
        std::cout << "Invalid parameters." << std::endl;
    }

    return boost::optional<po::variables_map>();
}

template <class T>
bool check_param(const char* param, const boost::program_options::variables_map& vm,
    const std::function<bool(T)>& checker, const char* error_msg)
{
    if (!vm.count(param))
    {
        std::cout << "Parameter '" << param << "' is not defined." << std::endl;
        return false;
    }
    else if (!checker(vm[param].as<T>()))
    {
        std::cout << error_msg << std::endl;
        return false;
    }

    return true;
}
}

boost::optional<Config> get_config(int argc, const char* const* argv)
{
    Config default_config{0, 0, std::thread::hardware_concurrency()};
    po::options_description desc = make_description(default_config);
    boost::optional<po::variables_map> vm = parse_arguments(argc, argv, desc);
    boost::optional<Config> empty_result;

    if (argc < 2 || !vm || vm.get().count("help"))
    {
        std::cout << desc << std::endl;
        return empty_result;
    }

    bool incomplete = !check_param<Port>("port", vm.get(),
        [](Port value) { return value >= 1024; }, "Parameter 'port' must be >= 1024.");
    incomplete = incomplete || !check_param<Clients>("clients", vm.get(),
        [](Clients value) { return value > 0; }, "Parameter 'clients' must be positive.");
    incomplete = incomplete || !check_param<Threads>("threads", vm.get(),
        [](Threads value) { return value > 0; }, "Parameter 'threads' must be positive.");

    if (incomplete)
    {
        return empty_result;
    }
    else if (default_config.clients < default_config.threads)
    {
        std::cout << "Number of threads(" << default_config.threads
            << ") can't exceed number of clients(" << default_config.clients << ")." << std::endl;
        return empty_result;
    }
    return default_config;
}
