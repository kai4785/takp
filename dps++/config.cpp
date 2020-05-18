#include "config.h"

using namespace std::literals::string_view_literals;

Config* configInstance()
{
    static Config config;
    static bool initialized = false;
    if(!initialized)
    {
        config.follow = false;
        config.history = false;
        config.reportByTarget = false;
        config.me = "You"sv;
        config.logfile = nullptr;
        config.since = -1;
        config.until = -1;
        config.keepAlive = 10;
        config.verbosity = 0;
        config.asio = false;
        initialized = true;
    }
    return &config;
}

