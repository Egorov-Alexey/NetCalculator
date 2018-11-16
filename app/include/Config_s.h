#pragma once

#include <string>

struct Config
{
	std::string address;
    unsigned short port;
    unsigned int clients;
    unsigned int threads;
};
