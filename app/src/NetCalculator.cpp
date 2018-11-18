#include <Config.h>
#include "NetCalcCore.h"

#include <iostream>
#include <boost/asio/signal_set.hpp>

/**
 * This is a main function for the application.
 * It creates and starts NetCalcCore instance.
 * It has 'event loop'.
 * Event loop waits SIGINT, SIGTERM signals.
 * When signal is received it stops NetCalcCore (stops its event loops).
 * User can break the application using Ctrl-C (SIGINT) keys or kill command (SIGTERM).
 */
int main(int argc, const char *argv[])
{
    //Parse result and make config.
    boost::optional<Config> config{get_config(argc, argv)};

    if (!config)
    {
        //Config is invalid.
        return 1;
    }

    try
    {
        NetCalcCore netCalcCore(config.get());
        netCalcCore.start();

        boost::asio::io_service service;
        boost::asio::signal_set sig(service, SIGINT, SIGTERM);
        sig.async_wait([&netCalcCore](const boost::system::error_code&, int){ netCalcCore.stop(); });
        service.run();
    }
    catch (const boost::system::system_error& e)
    {
        std::cerr << "Incought exception: " << e.what() << std::endl;
        return 2;
    }

    return 0;
}
