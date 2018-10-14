#include "Calculator.h"

#include <iostream>

Calculator::Calculator(const Config& cfg_)
    : cfg(cfg_),
      acceptor(service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), cfg.port))
{
    //Init clients.
    clients.reserve(cfg.clients);

    for (unsigned int i = 0; i < cfg.clients; ++i)
    {
        clients.push_back(client{boost::asio::ip::tcp::socket(service), {}, {}});
    }
}

Calculator::~Calculator()
{
    unsigned int thread_count = static_cast<unsigned int>(threads.size());
    for (unsigned int i = 0; i < thread_count; ++i)
    {
        std::thread& t = threads[i];
        if (t.joinable())
        {
            t.join();
        }
    }

    for (unsigned int i = 0; i < cfg.clients; ++i)
    {
        client& c = clients[i];
        if (c.socket.is_open())
        {
            c.socket.close();
        }
    }
}

void Calculator::start(bool block /*= true*/)
{
    for (unsigned int i = 0; i < cfg.clients; ++i)
    {
        dispatch_async_accept(i);
    }

    unsigned int threads_count = block ? cfg.threads - 1 : cfg.threads;
    threads.reserve(threads_count);
    for (unsigned int i = 0; i < threads_count; ++i)
    {
        threads.push_back(std::thread([&self = *this](){ self.service.run();}));
    }

    if (block)
    {
        service.run();
    }
}

void Calculator::stop()
{
    service.stop();
}

void Calculator::handle_accept(unsigned int client_index, const boost::system::error_code& error)
{
    if (error)
    {
//#ifdef DEBUG
        std::cerr << "Error " << error.value() << " on async_accept happens" << std::endl;
//#endif
        return;
    }

//#ifdef DEBUG
    std::cout << "Client: " << client_index << " accepted" << std::endl;
//#endif
    dispatch_async_receive(client_index);
}

void Calculator::handle_receive(unsigned int client_index, const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if (error)
    {
        client& c = clients[client_index];
        c.shunting_yard.clear();
        c.socket.close();

        dispatch_async_accept(client_index);
#ifdef DEBUG
        std::cerr << "Error " << error.value()
                  << " on async_receive happens for client: " << client_index << std::endl;
#endif
        return;
    }
#ifdef DEBUG
    std::string s(clients[client_index].buffer, bytes_transferred);
    std::cout << "Client: " << client_index << ", received: " << s << std::endl;
#endif
    client& c = clients[client_index];

    ShuntingYard::Result parse_result{c.shunting_yard.parse(c.buffer, bytes_transferred)};
    std::string str;

    switch (parse_result.first)
    {
        case ShuntingYard::ParseResult::Success:
            str = std::to_string(parse_result.second);
            c.socket.write_some(boost::asio::buffer(str));
        case ShuntingYard::ParseResult::Incomplete:
            dispatch_async_receive(client_index);
            return;
        case ShuntingYard::ParseResult::DivisionByZero:
            str = "Division by zero";
        case ShuntingYard::ParseResult::InvalidExpression:
            str = "Invalid expression";
    }

    c.socket.write_some(boost::asio::buffer(str));
    c.socket.close();
    dispatch_async_accept(client_index);
}

void Calculator::dispatch_async_accept(unsigned int client_index)
{
    auto l = [client_index, &self = *this](const boost::system::error_code& error)
    {
        self.handle_accept(client_index, error);
    };

    acceptor.async_accept(clients[client_index].socket, l);
}

void Calculator::dispatch_async_receive(unsigned int client_index)
{
    auto l = [client_index, &self = *this](const boost::system::error_code& error, std::size_t bytes_transferred)
    {
        self.handle_receive(client_index, error, bytes_transferred);
    };

    client& c = clients[client_index];
    c.socket.async_receive(boost::asio::buffer(c.buffer), l);
}
