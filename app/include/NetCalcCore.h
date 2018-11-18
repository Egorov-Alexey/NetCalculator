#pragma once

#include "Config.h"
#include <ShuntingYard.h>

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <boost/asio.hpp>

/**
 * This class implements net part (server part) of NetCalculator.
 *
 *  This class:
 *   - supports TCP/IPv4-connections;
 *   - calculates arithmetic expressions that receives from socket, calculates result and sends it back;
 *   - considers '\n' as end of expression;
 *   - can receive unified expression during several receive operations;
 *   - doesn't close connection after sending correct result;
 *   - sends string "Division by zero\n" if it happens and closes a connection;
 *   - sends string "Invalid expression\n" if it happens and closes a connection;
 *   - can process several simultaneous connections (depend on cfg_.clients parameter);
 *   - can start several threads (depend on cfg_.threads parameter and 'block' arguments of 'start' method);
 *   - implements event-driven approach (asynchronous model);
 *   - uses boost::asio.
 *
 * How to use it?
 *
 * 1. Provide config:
 * Config config;
 * 2. Create NetCalcCore instance:
 * NetCalcCore core(config).
 * 3. Start it in block or non-block mode:
 * core.start(true); //Current thread will be blocked.
 * 4. NetCalcCore proceeds incoming connections.
 * 5. Stop it from different thread or from signal handler:
 * core.stop();
 */
class NetCalcCore
{
public:
    /**
     * @brief This method constructs NetCalcCore object but doesn't start it.
     * @param cfg_[in] server configuration.
     */
    NetCalcCore(const Config& cfg_);

    /**
     * @brief This method joins started threads and closes open sockets.
     */
    ~NetCalcCore();

    /**
     * This method:
     *   - calls dispatch_async_accept cfg.clients times;
     *   - starts cfg.threads (if block is false) or cfg.threads - 1 (if block is true) threads.
     * @param block[in] if true current thread (in which start method is called) will be 'event loop'.
     */
    void start(bool block = false);

    /**
     * @brief This method calls service.stop() and stops event loops.
     */
    void stop();

private:
    using ShuntingYardInt = ShuntingYard<int>;

    /**
     * @brief This enum is used in unit-test mode to represent last async operation.
     */
    enum class client_unit_test_mode
    {
        none,
        async_accept,
        async_receive,
        async_send
    };

    /**
     * This struct represent one incoming connection.
     */
    struct client
    {
        //Socket object.
        boost::asio::ip::tcp::socket socket;
        //Buffer for sending/receiving data.
        char buffer[8192];
        //Object to compute a receiving expression.
        ShuntingYardInt shunting_yard;
        //Last async operation in unit-test mode.
        client_unit_test_mode unit_test_mode;
    };

private:
    /**
     * @brief Handle of 'async accept' operation.
     * It calls dispatch_async_receive() for successful result.
     * @param client_index[in] index of client, point at clients[client_index] object.
     * @param error[in] represents operating system-specific errors.
     */
    void handle_accept(unsigned int client_index, const boost::system::error_code& error);

    /**
     * @brief Handle of 'async receive' operation.
     * clients[client_index].buffer contains received data for successful result.
     * @param client_index[in] index of client, point at clients[client_index] object.
     * @param error[in] represents operating system-specific errors.
     * @param bytes_transferred[in] number of recived bytes.
     */
    void handle_receive(unsigned int client_index, const boost::system::error_code& error, std::size_t bytes_transferred);

    /**
     * @brief <handle of unsuccessful 'async receive' operation (it is called from handle_receive()).
     * @param client_index[in] index of client, point at clients[client_index] object.
     * @param error[in] represents operating system-specific errors.
     */
    void on_receive_error(unsigned int client_index, const boost::system::error_code& error);

    /**
     * @brief Handle of 'async send' operation.
     * @param client_index[in] index of client, point at clients[client_index] object.
     * @param processing_error[in] true if processing error (division by zero/invalid expression) happens during last receive.
     * @param bytes_transferred[in] number of sended bytes.
     * @param error[in] represents operating system-specific errors.
     */
    void handle_send(unsigned int client_index, bool processing_error, const boost::system::error_code& error, std::size_t bytes_transferred);

    /**
     * @brief Handle of unsuccessful 'async send' operation (it is called from handle_send()).
     * @param client_index[in] index of client, point at clients[client_index] object.
     * @param error[in] represents operating system-specific errors;
     */
    void on_send_error(unsigned int client_index, bool processing_error, const boost::system::error_code& error);

    /**
     * @brief Starts async accept operation (waits incomming connection).
     * @param client_index[in] index of client, point at clients[client_index] object.
     */
    void dispatch_async_accept(unsigned int client_index);

    /**
     * @brief Starts async receive operation (waits incomming data).
     * @param client_index[in] index of client, point at clients[client_index] object.
     */
    void dispatch_async_receive(unsigned int client_index);

    /**
     * @brief Starts async send operation (uses composed operation 'async_write).
     * @param client_index[in] index of client, point at clients[client_index] object.
     * @param bytes_to_write[in] how many bytes from clients[client_index].buffer need to send.
     * @param processing_error[in] pass true if server need to close connection after send data and do async accept.
     */
    void dispatch_async_send(unsigned int client_index, std::size_t bytes_to_write, bool processing_error);

    /**
     * @brief Parses received data and dispatch next async operation.
     * @param client_index[in] index of client, point at clients[client_index] object.
     * @param bytes_transferred[in] number of received bytes.
     */
    void parse_result(unsigned int client_index, std::size_t bytes_transferred);

private:
    //Provided server configuration (listen address, listen port, maximum number of NetCalcCore, number of threads).
    Config cfg;

    //Main boost::asio object.
    boost::asio::io_service service;

    //Object to accept incoming connections.
    boost::asio::ip::tcp::acceptor acceptor;

    //Container of objects to process multiple simultaneous connections (can't be empty).
    std::vector<client> clients;

    //Container of objects of started threads (can be empty).
    std::vector<std::thread> threads;

    /**
     * Flag of unit-test mode.
     * In this mode:
     *   - dispatch-methods don't do real operations;
     *   - set flag of last dispatch-method to clients[current_index].unit_test_mode.
     */
    bool unit_test_mode;

    //Friend for unit-tests.
    friend class NetCalcCoreTest;
};
