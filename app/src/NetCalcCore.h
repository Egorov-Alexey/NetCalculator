#pragma once

#include "Config.h"
#include "ShuntingYard.h"

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <boost/asio.hpp>

class NetCalcCore
{
public:
    NetCalcCore(const Config& cfg_);
    ~NetCalcCore();

    void start(bool block = false);
    void stop();

private:
    struct client
    {
        boost::asio::ip::tcp::socket socket;
        char buffer[8192];
        ShuntingYard shunting_yard;
    };

private:
    void handle_accept(unsigned int client_index, const boost::system::error_code& error);
    void handle_receive(unsigned int client_index, const boost::system::error_code& error, std::size_t bytes_transferred);

    void dispatch_async_accept(unsigned int client_index);
    void dispatch_async_receive(unsigned int client_index);

private:
    Config cfg;
    boost::asio::io_service service;
    boost::asio::ip::tcp::acceptor acceptor;
    std::vector<client> clients;
    std::vector<std::thread> threads;
};
