#include "tail.h"
#include "config.h"
#include "date.h"

#include <string.h>
#include <stdio.h>

 __attribute((constructor)) void configInstance(void)
{
    config.follow = false;
    config.history = true;
    config.me = NULL;
    config.logfile = NULL;
    config.since = NULL;
    config.keepalive = 10;
}

void tellme(const char* line)
{
    char datestring[25] = {0};
    const char* action = NULL;
    static ssize_t lineno = 0;
    int64_t dateseconds = 0;
    if(line[0] == '[' && line[25] == ']')
    {
        strncpy(datestring, &line[1], 24);
        action = &line[27];
        dateseconds = parseDate(datestring);
    }
    printf("[%zd]:[%s (%ld)] %s\n", ++lineno, datestring, dateseconds, action);
}

int main(int argc, char **argv)
{
    tail(argv[1], tellme);
    return 0;
}
