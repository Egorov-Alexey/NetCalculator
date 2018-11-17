#include "NetCalcCore.h"

#include <iostream>

NetCalcCore::NetCalcCore(const Config& cfg_)
    : cfg(cfg_),
      acceptor(service, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(cfg.address), cfg.port)),
      unit_test_mode(false)
{
    //Init clients.
    clients.reserve(cfg.clients);

    for (unsigned int i = 0; i < cfg.clients; ++i)
    {
        clients.push_back(client{boost::asio::ip::tcp::socket(service), {}, {}, {}});
    }
}

NetCalcCore::~NetCalcCore()
{
    //Join threads.
    unsigned int thread_count = static_cast<unsigned int>(threads.size());
    for (unsigned int i = 0; i < thread_count; ++i)
    {
        std::thread& t = threads[i];
        if (t.joinable())
        {
            t.join();
        }
    }

    //Close all sockets.
    for (unsigned int i = 0; i < cfg.clients; ++i)
    {
        client& c = clients[i];
        if (c.socket.is_open())
        {
            c.socket.close();
        }
    }
}

void NetCalcCore::start(bool block /*= false*/)
{
    //Dispatch async operation for each client.
    for (unsigned int i = 0; i < cfg.clients; ++i)
    {
        dispatch_async_accept(i);
    }

    //Start cfg.threads or cfg.threads - 1 threads.
    //Each started thread will be 'event loop'.
    unsigned int threads_count = block ? cfg.threads - 1 : cfg.threads;
    threads.reserve(threads_count);
    for (unsigned int i = 0; i < threads_count; ++i)
    {
        threads.push_back(std::thread([&self = *this](){ self.service.run();}));
    }

    //Current thread will be 'event loop' if block is true.
    if (block)
    {
        service.run();
    }
}

void NetCalcCore::stop()
{
    //Stop all event loop.
    service.stop();
}

void NetCalcCore::handle_accept(unsigned int client_index, const boost::system::error_code& error)
{
    if (error)
    {
#ifndef NDEBUG
        std::cerr << "Error " << error.value() << " on async_accept happens" << std::endl;
#endif
        return;
    }

#ifndef NDEBUG
    std::cout << "Client: " << client_index << " accepted" << std::endl;
#endif

    //Dispatch async receive for successful accept.
    dispatch_async_receive(client_index);
}

void NetCalcCore::handle_receive(unsigned int client_index, const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if (error)
    {
        on_receive_error(client_index, error);
        return;
    }

#ifndef NDEBUG
    std::cout << "Client: " << client_index << ", received: " << bytes_transferred << " bytes" << std::endl;
#endif

    //Parse received data and dispatch next async operation.
    parse_result(client_index, bytes_transferred);
}

void NetCalcCore::on_receive_error(unsigned int client_index, const boost::system::error_code& error)
{
    //Clear client object, close connection and dispatch async accept.
    client& c = clients[client_index];
    c.shunting_yard.clear();
    c.socket.close();

#ifndef NDEBUG
    std::cerr << "Error " << error.value()
              << " on async_receive happens for client: " << client_index << std::endl;
#endif

    dispatch_async_accept(client_index);
}

void NetCalcCore::handle_send(unsigned int client_index, bool processing_error, const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if (error || processing_error)
    {
        on_send_error(client_index, processing_error, error);
        return;
    }

#ifndef NDEBUG
    std::cout << "Client: " << client_index << ", writen: " << bytes_transferred << " bytes" << std::endl;
#endif

    if (!clients[client_index].shunting_yard.is_empty())
    {
        //Long expression like: '1 + 2\n3 - 4\n5 * 6\n7 / 8\n' was received.
        parse_result(client_index, 0ul);
    }
    else
    {
        //Dispatch async receive to receive next arithmetic expression.
        dispatch_async_receive(client_index);
    }
}

void NetCalcCore::on_send_error(unsigned int client_index, bool processing_error, const boost::system::error_code& error)
{
    //Clear client object, close connection and dispatch async accept.
    client& c = clients[client_index];
    c.shunting_yard.clear();
    c.socket.close();

#ifndef NDEBUG
    if (processing_error)
    {
        std::cerr << "Processing error happens for client: " << client_index << std::endl;
    }
    else
    {
        std::cerr << "Error " << error.value()
                  << " on async_write happens for client: " << client_index << std::endl;
    }
#endif

    dispatch_async_accept(client_index);
}

void NetCalcCore::dispatch_async_accept(unsigned int client_index)
{
    auto l = [client_index, &self = *this](const boost::system::error_code& error)
    {
        self.handle_accept(client_index, error);
    };

    client& c = clients[client_index];
    if (unit_test_mode)
        c.unit_test_mode = client_unit_test_mode::async_accept;
    else
        acceptor.async_accept(c.socket, l);
}

void NetCalcCore::dispatch_async_receive(unsigned int client_index)
{
    auto l = [client_index, &self = *this](const boost::system::error_code& error, std::size_t bytes_transferred)
    {
        self.handle_receive(client_index, error, bytes_transferred);
    };

    client& c = clients[client_index];
    if (unit_test_mode)
        c.unit_test_mode = client_unit_test_mode::async_receive;
    else
        c.socket.async_receive(boost::asio::buffer(c.buffer), l);
}

void NetCalcCore::dispatch_async_send(unsigned int client_index, std::size_t bytes_to_write, bool processing_error)
{
    auto l = [client_index, processing_error, &self = *this](const boost::system::error_code& error, std::size_t bytes_transferred)
    {
        self.handle_send(client_index, processing_error, error, bytes_transferred);
    };

    client& c = clients[client_index];
    if (unit_test_mode)
        c.unit_test_mode = client_unit_test_mode::async_send;
    else
        boost::asio::async_write(c.socket, boost::asio::buffer(c.buffer, bytes_to_write), l);
}

void NetCalcCore::parse_result(unsigned int client_index, std::size_t bytes_transferred)
{
    //bytes_transferred == 0 means that long expression like: '1 + 2\n3 - 4\n5 * 6\n7 / 8\n' was received.
    //It is need to calculate next result.
    std::string str;
    bool processing_error = false;
    client& c = clients[client_index];
    ShuntingYard::Result parse_result = c.shunting_yard.parse(bytes_transferred ? c.buffer : nullptr, bytes_transferred);

    switch (parse_result.first)
    {
        case ShuntingYard::ParseResult::Success:
            str = std::to_string(parse_result.second) + "\n";
            break;
        case ShuntingYard::ParseResult::Incomplete:
            break;
        case ShuntingYard::ParseResult::DivisionByZero:
            str = "Division by zero\n";
            processing_error = true;
            break;
        case ShuntingYard::ParseResult::InvalidExpression:
            str = "Invalid expression\n";
            processing_error = true;
            break;
    }

    if (str.empty())
    {
        dispatch_async_receive(client_index);
    }
    else
    {
        memcpy(c.buffer, str.data(), str.size());
        dispatch_async_send(client_index, str.size(), processing_error);
    }
}
