#pragma once

#include <string>
#include <boost/optional.hpp>

struct Config
{
    // Listen address.
    std::string address;

    //Listen port.
    unsigned short port;

    //Maximum number of simultaneous clients.
    unsigned int clients;

    //Number of threads (can't exceed 'clients' field).
    unsigned int threads;
};

/**
 * This function parses arguments of the 'main' function and returns Config.
 * Developer needs to delegate argc/argv parameters from main to this funcion.
 *
 * This function prints (to std::cout) a description of argiments if:
 *    - argc/argv don't contain arguments (only a program name is provided);
 *    - an arguments processing error happens;
 *    - 'help' parameter provided;
 *
 * This function would print (to std::cout) an error message if an incorrect value was provided.
 *
 * argc/argv parameters meaning:
 * -a or --address means 'Listen address' (optional parameter);
 * -p or --port means 'Listen port' (mandatory parameter);
 * -c or --clients means 'Listen address' (mandatory parameter);
 * -t or --threads means 'Number of threads' (optional parameter);
 *
 * Default value for address is '127.0.0.1'.
 * Default value for threads is std::thread::hardware_concurrency() or 1 (if value is not computable).
 *
 * @param argc[in] argc argument from main;
 * @param argv[in] argv argument from mian;
 * @return inited Config if argc/argv contains valid values else not inited Config.
 */
boost::optional<Config> get_config(int argc, const char* const* argv);
