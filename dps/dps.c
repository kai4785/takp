#include "action.h"
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
    config.since = 0;
    config.keepalive = 10;
}

void tellme(struct String line)
{
    static int64_t lineno = 0;
    lineno++;
    // empty line
    if(line.length == 0)
        return;
    char datestring[25] = {0};
    struct String message = {0};
    int64_t dateseconds = 0;

    if(line.length < 27 || !(line.data[0] == '[' && line.data[25] == ']'))
    {
        fprintf(stderr, "[%ld] No date in message(%zu): (%02x %02x) [|%s|]\n", lineno, line.length, line.data[0], line.data[25], line.data);
        return;
    }
    strncpy(datestring, &line.data[1], 24);
    message.data = &line.data[27];
    message.length = line.length - 27;
    dateseconds = parseDate(datestring);
    if(dateseconds < config.since)
    {
        return;
    }
    struct Action action = parseAction(message);
    switch(action.type)
    {
        case CHAT:
        {
            fprintf(stderr, "%s\n", message.data);
            break;
        }
        case MELEE:
        {
            printf("Melee: %s:\n", message.data);
            printf("      ");
            printf(" %.*s", (int)action.source.length, action.source.data);
            printf("|%.*s", (int)action.verb.length, action.verb.data);
            printf("|%.*s", (int)action.target.length, action.target.data);
            printf("|%ld", action.damage);
            printf("\n");
            break;
        }
        case MAGIC:
        {
            printf("Magic: %s:\n", message.data);
            printf("      ");
            printf(" %.*s", (int)action.target.length, action.target.data);
            printf("|%.*s", (int)action.verb.length, action.verb.data);
            printf("|%ld", action.damage);
            printf("\n");
            break;
        }
        case HEAL:
        {
            printf("Heal : %s:\n", message.data);
            printf("      ");
            printf(" %.*s", (int)action.target.length, action.target.data);
            printf("|%.*s", (int)action.verb.length, action.verb.data);
            printf("|%ld", action.damage);
            printf("\n");
            break;
        }
        default:
        {
            fprintf(stderr, "[%zd]:[%ld] %s\n", lineno, dateseconds, message.data);
            break;
        }
    }
}

int main(int argc, char **argv)
{
    if(argc > 2)
    {
        config.since = parseDate(argv[2]);
    }
    tail(argv[1], tellme);
    return 0;
}
