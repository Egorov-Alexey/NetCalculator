#pragma once

#include <boost/optional.hpp>

struct Config
{
    unsigned int port;
    unsigned int clients;
    unsigned int threads;
};

boost::optional<Config> get_config(int argc, const char *argv[]);
