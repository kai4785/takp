#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

struct Config
{
    bool follow;
    bool history;
    const char* me;
    const char* logfile;
    const char* since;
    int keepalive;
};

extern struct Config config;

#endif // CONFIG_H
