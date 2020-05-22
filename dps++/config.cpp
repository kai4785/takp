#include "config.h"

using namespace std::literals::string_view_literals;

Config& configInstance()
{
    static Config config = {
        false,
        false,
        false,
        0,
        "You"sv,
        nullptr,
        -1,
        -1,
        10,
        Config::IO_LOOP,
    };
    return config;
}

