#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <inttypes.h>

struct Config
{
    bool follow;
    bool history;
    int verbosity;
    const char* me;
    const char* logfile;
    uint64_t since;
    int keepAlive;
};

extern struct Config config;

#endif // CONFIG_H
