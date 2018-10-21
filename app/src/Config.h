#pragma once

#include <boost/optional.hpp>

struct Config
{
    unsigned short port;
    unsigned int clients;
    unsigned int threads;
};

boost::optional<Config> get_config(int argc, const char *argv[]);
