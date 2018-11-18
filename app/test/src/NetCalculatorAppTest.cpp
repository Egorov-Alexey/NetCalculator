/**
 * This file contains unit-tests for whole NetCalculator application.
 * Start NetCalculator application for this test.
 * main function returns 0 if all test passed.
 * main function returns 1 if one of test failed.
 */

#include <iostream>

#include <boost/asio.hpp>
#include <boost/optional.hpp>

struct TestCase
{
    bool need_conect;
    bool need_disconnect;
    std::string request;
    std::string response;
};

const TestCase test_cases[] =
{
    {true,  false, "1 + 2\n", "3\n"},
    {false, false, "3 + 4\n", "7\n"},
    {false, true,  "(1 + 2\n", "Invalid expression\n"},
    {true,  true,  "5/(3/7)\n", "Division by zero\n"},
    {true,  true,  "(2 + 3) * 7 / 11\n(109 - 53) * 17 / 19\n103/((67 - 43) / 7)\n", "3\n50\n34\n"}
};

template <typename SyncReadStream, typename MutableBufferSequence>
size_t readWithTimeout(SyncReadStream& s,
    const MutableBufferSequence& buffers,
    boost::asio::deadline_timer& timer,
    boost::system::error_code& error)
{
    boost::optional<boost::system::error_code> timer_result;
    timer.async_wait([&timer_result] (const boost::system::error_code& error) { timer_result.reset(error); });

    size_t readed = 0;
    boost::optional<boost::system::error_code> read_result;
    boost::asio::async_read(s, buffers, [&read_result, &readed] (const boost::system::error_code& error, size_t bytes_received) { read_result.reset(error); readed = bytes_received; });

    s.get_io_service().reset();
    while (s.get_io_service().run_one())
    {
        if (read_result)
            timer.cancel();
        else if (timer_result)
            s.cancel();
    }

    if (*read_result)
    {
        error = *read_result;
        return 0;
    }

    return readed;
}

class NetCalculatorAppTest
{
public:
    NetCalculatorAppTest(int argc, const char* const* argv) :
        endpoint(make_endpoint(argc, argv)),
        socket(service) {}

    ~NetCalculatorAppTest() { if (socket.is_open()) socket.close(); }

    bool connect();
    bool exchange(const std::string& request, std::string& response, size_t need_to_read);
    void disconnect() { if (socket.is_open()) socket.close();}

private:
    static boost::asio::ip::tcp::endpoint make_endpoint(int argc, const char* const* argv);

private:
    boost::asio::io_service service;
    boost::asio::ip::tcp::endpoint endpoint;
    boost::asio::ip::tcp::socket socket;
};

boost::asio::ip::tcp::endpoint NetCalculatorAppTest::make_endpoint(int argc, const char* const* argv)
{
    if (argc != 3)
    {
        throw std::runtime_error("Invalid arguments.");
    }

    boost::system::error_code ec;
    boost::asio::ip::address address = boost::asio::ip::address::from_string(argv[1], ec);
    if (ec)
    {
        throw std::runtime_error("Invalid arguments.");
    }

    return boost::asio::ip::tcp::endpoint(address, std::atoi(argv[2]));
}

bool NetCalculatorAppTest::connect()
{
    boost::system::error_code ec;
    socket.connect(endpoint, ec);

    if (ec)
    {
        std::cerr << "Test failed. Could not connect to host" << std::endl;
        return false;
    }

    return true;
}

bool NetCalculatorAppTest::exchange(const std::string& request, std::string& response, size_t need_to_read)
{
    boost::system::error_code ec;
    boost::asio::write(socket, boost::asio::buffer(request), boost::asio::transfer_all(), ec);
    if (ec)
    {
        std::cerr << "Test failed. Could not send request." << std::endl;
        return false;
    }

    std::vector<char> response_buffer(need_to_read, 0);
    size_t readed = 0;

    boost::asio::deadline_timer timer(service);
    timer.expires_from_now(boost::posix_time::seconds(5));
    while (!ec && (need_to_read - readed))
    {
        size_t n = readWithTimeout(socket, boost::asio::buffer(&response_buffer[readed], need_to_read - readed), timer, ec);
        if (!ec)
        {
            readed += n;
        }
    }

    if (ec)
    {
        std::cerr << "Test failed. Could not get response." << std::endl;
        return false;
    }

    response.assign(response_buffer.data(), readed);

    return true;
}

int main(int argc, char* argv[])
{
    try
    {
        NetCalculatorAppTest test(argc, argv);

        for (const TestCase& test_case : test_cases)
        {
            if (test_case.need_conect && !test.connect())
            {
                return 1;
            }

            std::string response;
            if (!test.exchange(test_case.request, response, test_case.response.size()))
            {
                return 1;
            }

            if (response != test_case.response)
            {
                std::cout << response << std::endl << test_case.response << std::endl;
                std::cerr << "Test failed. Invalid response received" << std::endl;
                return 1;
            }

            if (test_case.need_disconnect)
            {
                test.disconnect();
            }
            std::cout << "Success" << std::endl;
        }
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Test failed. " <<  e.what() << std::endl;
        return 1;
    }

    return 0;
}
