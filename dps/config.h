#ifndef CONFIG_H
#define CONFIG_H

#include "utility.h"

#include <stdbool.h>
#include <inttypes.h>

struct Config
{
    bool follow;
    bool history;
    int verbosity;
    struct SimpleString me;
    const char* logfile;
    int64_t since;
    int keepAlive;
};

struct Config* configInstance();

#endif // CONFIG_H
