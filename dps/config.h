#ifndef CONFIG_H
#define CONFIG_H

#include "utility.h"

#include <stdbool.h>
#include <inttypes.h>

struct Config
{
    bool follow;
    bool history;
    bool reportByTarget;
    bool extra;
    int verbosity;
    struct SimpleString me;
    const char* logfile;
    int64_t since;
    int64_t until;
    int keepAlive;
};

struct Config* configInstance();

#endif // CONFIG_H
