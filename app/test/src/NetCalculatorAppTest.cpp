#include <iostream>

#include <boost/asio.hpp>
#include <boost/optional.hpp>

struct Socket
{
    ~Socket() { if (s.is_open()) s.close(); }
    boost::asio::ip::tcp::socket s;
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

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Test failed. Invalid arguments. " << std::endl;
        return 1;
    }

    boost::system::error_code ec;
    boost::asio::ip::address address = boost::asio::ip::address::from_string(argv[1], ec);
    if (ec)
    {
        std::cerr << "Test failed. Invalid address: " << argv[1] << std::endl;
        return 1;
    }

    boost::asio::io_service service;
    boost::asio::ip::tcp::endpoint endpoint(address, std::atoi(argv[2]));
    Socket socket{boost::asio::ip::tcp::socket{service}};

    socket.s.connect(endpoint, ec);
    if (ec)
    {
        std::cerr << "Test failed. Could not connect to " << argv[1] << ":" << std::atoi(argv[2]) << std::endl;
        return 1;
    }

    std::string request = "(2 + 3) * 7 / 11\n(109 - 53) * 17 / 19\n103/((67 - 43) / 7)\n";
    boost::asio::write(socket.s, boost::asio::buffer(request), boost::asio::transfer_all(), ec);
    if (ec)
    {
        std::cerr << "Test failed. Could not send request." << std::endl;
        return 1;
    }

    const std::string expected_response{"3\n50\n34\n"};

    char response_buffer[8];
    size_t readed = 0;
    size_t need_to_read = sizeof(response_buffer);

    boost::asio::deadline_timer timer(service);
    timer.expires_from_now(boost::posix_time::seconds(5));
    while (!ec && (need_to_read - readed))
    {
        size_t n = readWithTimeout(socket.s, boost::asio::buffer(&response_buffer[readed], need_to_read - readed), timer, ec);
        if (!ec)
        {
            readed += n;
        }
    }

    if (ec)
    {
        std::cerr << "Test failed. Could not get response." << std::endl;
        return 1;
    }

    if (expected_response != response_buffer)
    {        
        std::cerr << "Test failed. Invalid data received" << std::endl;
        return 1;
    }

    std::cout << "Test passed" << std::endl;
    return 0;
}

