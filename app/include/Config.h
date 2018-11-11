#pragma once

#include <Config_s.h>

#include <boost/optional.hpp>

boost::optional<Config> get_config(int argc, const char* const* argv);
