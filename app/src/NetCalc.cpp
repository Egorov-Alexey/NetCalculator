#include "Config.h"
#include "Calculator.h"

int main(int argc, const char *argv[])
{
    boost::optional<Config> config{get_config(argc, argv)};

    if (!config)
    {
        return 1;
    }

    Calculator c(config.get());
    c.start();

	return 0;
}
