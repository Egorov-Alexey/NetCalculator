#pragma once

#include <Config_s.h>
#include <ShuntingYard.h>

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
    enum class client_unit_test_mode
    {
        none,
        async_accept,
        async_receive,
        async_send
    };

    struct client
    {
        boost::asio::ip::tcp::socket socket;
        char buffer[8192];
        ShuntingYard shunting_yard;
        client_unit_test_mode unit_test_mode;
    };

private:
    void handle_accept(unsigned int client_index, const boost::system::error_code& error);
    void handle_receive(unsigned int client_index, const boost::system::error_code& error, std::size_t bytes_transferred);
    void on_receive_error(unsigned int client_index, const boost::system::error_code& error);
    void handle_send(unsigned int client_index, bool processing_error, const boost::system::error_code& error, std::size_t bytes_transferred);
    void on_send_error(unsigned int client_index, bool processing_error, const boost::system::error_code& error);

    void dispatch_async_accept(unsigned int client_index);
    void dispatch_async_receive(unsigned int client_index);
    void dispatch_async_send(unsigned int client_index, std::size_t bytes_to_write, bool processing_error);

    void parse_result(unsigned int client_index, std::size_t bytes_transferred);

private:
    Config cfg;
    boost::asio::io_service service;
    boost::asio::ip::tcp::acceptor acceptor;
    std::vector<client> clients;
    std::vector<std::thread> threads;
    bool unit_test_mode;

    //Friend for unit-tests.
    friend class NetCalcCoreTest;
};
