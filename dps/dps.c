#include "action.h"
#include "battle.h"
#include "config.h"
#include "date.h"
#include "tail.h"

#include <string.h>
#include <stdio.h>

// TODO: Really?
 __attribute((constructor)) void configInstance(void)
{
    config.follow = false;
    config.history = true;
    config.me = NULL;
    config.logfile = NULL;
    config.since = 0;
    config.keepalive = 10;
    config.verbose = 0;
}

void tellme(struct String line)
{
    static int64_t lineno = 0;
    lineno++;
    // empty line
    if(line.length == 0)
        return;
    struct String datestring = {0};
    struct String message = {0};
    int64_t dateseconds = 0;

    if(line.length < 27 || !(line.data[0] == '[' && line.data[25] == ']'))
    {
        fprintf(stderr, "[%ld] No date in message(%zu): (%02x %02x) [|%s|]\n", lineno, line.length, line.data[0], line.data[25], line.data);
        return;
    }
    datestring.data = line.data + 1;
    datestring.length = 24;
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
            battle.totalDamage += action.damage;
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
            battle.totalDamage += action.damage;
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
            battle.totalHeals += action.damage;
            break;
        }
        default:
        {
            if(config.verbose)
                fprintf(stderr, "[%zd]:[%ld] %s\n", lineno, dateseconds, message.data);
            break;
        }
    }
}

int main(int argc, char **argv)
{
    if(argc > 2)
    {
        struct String datestring = {
            .data = argv[2],
            .length = strlen(argv[2])
        };
        config.since = parseDate(datestring);
    }
    tail(argv[1], tellme);
    reportBattle();
    return 0;
}
