#include "NetCalcCore.h"

#include <iostream>

NetCalcCore::NetCalcCore(const Config& cfg_)
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

NetCalcCore::~NetCalcCore()
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

void NetCalcCore::start(bool block /*= false*/)
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

void NetCalcCore::stop()
{
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
    dispatch_async_receive(client_index);
}

void NetCalcCore::handle_receive(unsigned int client_index, const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if (error)
    {
        client& c = clients[client_index];
        c.shunting_yard.clear();
        c.socket.close();

        dispatch_async_accept(client_index);
#ifndef NDEBUG
        std::cerr << "Error " << error.value()
                  << " on async_receive happens for client: " << client_index << std::endl;
#endif
        return;
    }
#ifndef NDEBUG
    std::cout << "Client: " << client_index << ", received: " << bytes_transferred << " bytes" << std::endl;
#endif
    client& c = clients[client_index];

    std::string str;
    //Long expression like: '1 + 2\n3 - 4\n5 * 6\n7 / 8\n' could be received.
    bool long_expression = false;

    do
    {
        ShuntingYard::Result parse_result{
            c.shunting_yard.parse(long_expression ? nullptr : c.buffer, long_expression ? 0ul : bytes_transferred)};

        switch (parse_result.first)
        {
            case ShuntingYard::ParseResult::Success:
            {
                str = std::to_string(parse_result.second) + "\n";
                boost::system::error_code ec;
                boost::asio::write(c.socket, boost::asio::buffer(str), boost::asio::transfer_all(), ec);
                if (ec)
                {
                    //An error occurred.
                    c.socket.close();
                    dispatch_async_accept(client_index);
                    return;
                }
				long_expression = !c.shunting_yard.is_empty();
				if (long_expression)
                {
                    //Continue processing long expression.
                    break;
                }
            }
            case ShuntingYard::ParseResult::Incomplete:
                dispatch_async_receive(client_index);
                return;
            case ShuntingYard::ParseResult::DivisionByZero:
                str = "Division by zero";
                break;
            case ShuntingYard::ParseResult::InvalidExpression:
                str = "Invalid expression";
        }
    } while (long_expression);

    c.socket.write_some(boost::asio::buffer(str));
    c.socket.close();
    dispatch_async_accept(client_index);
}

void NetCalcCore::dispatch_async_accept(unsigned int client_index)
{
    auto l = [client_index, &self = *this](const boost::system::error_code& error)
    {
        self.handle_accept(client_index, error);
    };

    acceptor.async_accept(clients[client_index].socket, l);
}

void NetCalcCore::dispatch_async_receive(unsigned int client_index)
{
    auto l = [client_index, &self = *this](const boost::system::error_code& error, std::size_t bytes_transferred)
    {
        self.handle_receive(client_index, error, bytes_transferred);
    };

    client& c = clients[client_index];
    c.socket.async_receive(boost::asio::buffer(c.buffer), l);
}
