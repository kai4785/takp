#ifndef CONFIG_H

#define CONFIG_H

#include <string_view>
#include <cstdint>

struct Config
{
    enum IoMode
    {
        IO_LOOP,
        IO_ASIO,
    };
    bool follow;
    bool history;
    bool reportByTarget;
    int verbosity;
    std::string_view me;
    std::string_view logfile;
    int64_t since;
    int64_t until;
    int keepAlive;
    IoMode io;
};

Config& configInstance();

#endif // CONFIG_H
