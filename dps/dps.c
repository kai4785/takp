#include "action.h"
#include "battle.h"
#include "config.h"
#include "date.h"
#include "tail.h"

#include <string.h>
#include <stdio.h>

// TODO: Really?
 __attribute((constructor)) void globalConstructor(void)
{
    config.follow = false;
    config.history = true;
    config.me = NULL;
    config.logfile = NULL;
    config.since = 0;
    config.keepAlive = 10;
    config.verbosity = 0;
    Battle_ctor(&battle);
}
 __attribute((destructor)) void globalDestructor(void)
{
    battle.dtor(&battle);
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

    if(battle.m_start > 0 && battle.m_end < dateseconds)
    {
        battle.report(&battle);
        battle.reset(&battle);
    }

    struct Action action;
    Action_ctor(&action);
    action.parse(&action, message);
    switch(action.type)
    {
        case CHAT:
        {
            fprintf(stderr, "%s\n", message.data);
            break;
        }
        case MELEE:
        {
            if(config.verbosity > 0)
            {
                printf("Melee: %s:\n", message.data);
                printf("      ");
                printf(" %.*s", (int)action.source.length, action.source.data);
                printf("|%.*s", (int)action.verb.length, action.verb.data);
                printf("|%.*s", (int)action.target.length, action.target.data);
                printf("|%ld", action.damage);
                printf("\n");
            }
            if(battle.m_start == 0)
                battle.start(&battle, dateseconds);
            battle.melee(&battle, &action);
            battle.m_end = dateseconds + config.keepAlive;
            break;
        }
        case MAGIC:
        {
            if(config.verbosity > 0)
            {
                printf("Magic: %s:\n", message.data);
                printf("      ");
                printf(" %.*s", (int)action.target.length, action.target.data);
                printf("|%.*s", (int)action.verb.length, action.verb.data);
                printf("|%ld", action.damage);
                printf("\n");
            }
            if(battle.m_start == 0)
                battle.start(&battle, dateseconds);
            battle.m_end = dateseconds + config.keepAlive;
            battle.m_totalDamage += action.damage;
            break;
        }
        case HEAL:
        {
            if(config.verbosity > 0)
            {
                printf("Heal : %s:\n", message.data);
                printf("      ");
                printf(" %.*s", (int)action.target.length, action.target.data);
                printf("|%.*s", (int)action.verb.length, action.verb.data);
                printf("|%ld", action.damage);
                printf("\n");
            }
            if(battle.m_start == 0)
                battle.start(&battle, dateseconds);
            battle.m_end = dateseconds + config.keepAlive;
            battle.m_totalHeals += action.damage;
            break;
        }
        case UNKNOWN:
        default:
        {
            if(config.verbosity > 1)
                fprintf(stderr, "[%zd]:[%ld] %s\n", lineno, dateseconds, message.data);
            break;
        }
    }
    action.dtor(&action);
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
    tail(argv[1], &tellme);
    printf("PCs: %zu\n", battle.m_pc.size);
    for(size_t i = 0; i < battle.m_pc.size; i++)
    {
        struct String pc = *(struct String*)battle.m_pc.at(&battle.m_pc, i);
        printf("PC[%zd]: %.*s\n", i, (int)pc.length, pc.data);
    }
    return 0;
}
