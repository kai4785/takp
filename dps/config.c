#include "config.h"

struct Config* configInstance()
{
    static struct Config config;
    static bool initialized = false;
    if(!initialized)
    {
        config.follow = false;
        config.history = false;
        config.reportByTarget = false;
        config.extra = false;
        config.misses = false;
        config.byVerb = false;
        config.me = (struct SimpleString)SIMPLE_STRING("You");
        config.logfile = NULL;
        config.since = -1;
        config.until = -1;
        config.keepAlive = 10;
        config.verbosity = 0;
        initialized = true;
    }
    return &config;
}

