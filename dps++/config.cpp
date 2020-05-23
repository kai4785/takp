#include "config.h"

using namespace std::literals::string_view_literals;

Config& Config::instance()
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

