#include "Config.h"
#include "NetCalcCore.h"

#include <boost/asio/signal_set.hpp>

int main(int argc, const char *argv[])
{
    boost::optional<Config> config{get_config(argc, argv)};

    if (!config)
    {
        return 1;
    }

    NetCalcCore netCalcCore(config.get());
    netCalcCore.start();

    boost::asio::io_service service;
    boost::asio::signal_set sig(service, SIGINT, SIGTERM);
    sig.async_wait([&netCalcCore](const boost::system::error_code&, int){ netCalcCore.stop(); });
    service.run();

	return 0;
}
