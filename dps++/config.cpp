#include "config.h"

using namespace std::literals::string_view_literals;

Config& Config::instance()
{
    static Config config = {
        false,           // follow
        false,           // history
        false,           // reportByTarget
        0,               // verbosity
        "You"sv,         // me
        ""sv,            // logfile
        -1,              // since
        -1,              // until
        10,              // keepalive
        Config::IO_LOOP, //iomode
    };
    return config;
}