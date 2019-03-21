#include "action.h"
#include "battle.h"
#include "config.h"
#include "date.h"
#include "tail.h"

#include <string.h>
#include <stdio.h>


void tellme(struct SimpleString line)
{
    struct Config* config = configInstance();
    static int64_t lineno = 0;
    lineno++;
    // empty line
    if(line.length == 0)
        return;
    int64_t dateseconds = 0;

    if(line.length < 27 || !(line.data[0] == '[' && line.data[25] == ']'))
    {
        fprintf(stderr, "[%"PRId64"] No date in message(%zu): (%02x %02x) [|%s|]\n", lineno, line.length, line.data[0], line.data[25], line.data);
        return;
    }
    struct SimpleString datestring = {
        .data = line.data + 1,
        .length = 24
    };
    struct String message;
    String_ctorHold(&message,
        &line.data[27],
        line.length - 27
    );
    dateseconds = parseDate(datestring);
    if(dateseconds < config->since)
    {
        return;
    }

    struct Battle* battle = battleInstance();
    if(battle->m_start > 0 && battle->m_end < dateseconds)
    {
        battle->report(battle);
        battle->reset(battle);
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
            if(config->verbosity > 0)
            {
                printf("Melee: %s:\n", message.data);
                printf("      ");
                printf(" %.*s", (int)action.source.length, action.source.data);
                printf("|%.*s", (int)action.verb.length, action.verb.data);
                printf("|%.*s", (int)action.target.length, action.target.data);
                printf("|%"PRId64"", action.damage);
                printf("\n");
            }
            if(battle->m_start == 0)
                battle->start(battle, dateseconds);
            battle->melee(battle, dateseconds, &action);
            battle->m_end = dateseconds + config->keepAlive;
            break;
        }
        case MAGIC:
        {
            if(config->verbosity > 0)
            {
                printf("Magic: %s:\n", message.data);
                printf("      ");
                printf(" %.*s", (int)action.target.length, action.target.data);
                printf("|%.*s", (int)action.verb.length, action.verb.data);
                printf("|%"PRId64"", action.damage);
                printf("\n");
            }
            if(battle->m_start == 0)
                battle->start(battle, dateseconds);
            battle->m_end = dateseconds + config->keepAlive;
            battle->m_totalDamage += action.damage;
            break;
        }
        case HEAL:
        {
            if(config->verbosity > 0)
            {
                printf("Heal : %s:\n", message.data);
                printf("      ");
                printf(" %.*s", (int)action.target.length, action.target.data);
                printf("|%.*s", (int)action.verb.length, action.verb.data);
                printf("|%"PRId64"", action.damage);
                printf("\n");
            }
            if(battle->m_start == 0)
                battle->start(battle, dateseconds);
            battle->m_end = dateseconds + config->keepAlive;
            battle->m_totalHeals += action.damage;
            break;
        }
        case UNKNOWN:
        default:
        {
            if(config->verbosity > 1)
                fprintf(stderr, "[%"PRId64"]:[%"PRId64"] %s\n", lineno, dateseconds, message.data);
            break;
        }
    }
    action.dtor(&action);
}

#if 0
From Python
('--me', '-m', help='Who is "You" in the logs', default='Me')
('--pc', '-p', action='append', help='Filter to specific (Non-)Player Characters to search for in the logs')
('--log', '-l', help='Logfile to watch')
('--history', help='Read the whole log history', action='store_true')
('--follow', '-f', help='Follow the log file', action='store_true')
('--since', '-s', help='Parse logs since', default='Thu Jan 01 00:00:00 1970')
('--keep-alive', '-k', help='Keep alive seconds for each Battle', default=10)
#endif
int main(int argc, char **argv)
{
    struct Config* config = configInstance();
    struct String opt_me = CONST_STRING("--me");
    struct String opt_log = CONST_STRING("--log");
    struct String opt_history = CONST_STRING("--history");
    struct String opt_follow = CONST_STRING("--follow");
    struct String opt_since = CONST_STRING("--since");
    struct String opt_keepalive = CONST_STRING("--keepalive");

    // Ew, manual parsing? Don't mess up, cause I'll just barf.
    char* logfile = NULL;
    for(int i = 1; i < argc; i++)
    {
        struct SimpleString arg = SIMPLE_STRING(argv[i]);
        if(opt_me.op_equal(&opt_me, &arg))
        {
            i++;
            config->me = (struct SimpleString)SIMPLE_STRING(argv[i]);
        }
        if(opt_log.op_equal(&opt_log, &arg))
        {
            i++;
            logfile = argv[i];
        }
        if(opt_history.op_equal(&opt_history, &arg))
        {
            config->history = true;
        }
        if(opt_follow.op_equal(&opt_follow, &arg))
        {
            config->follow = true;
        }
        if(opt_since.op_equal(&opt_since, &arg))
        {
            i++;
            struct SimpleString datestring = (struct SimpleString)SIMPLE_STRING(argv[i]);
            config->since = parseDate(datestring);
            if(config->since < 0)
            {
                fprintf(stderr, "Error parsing --since string '%s'\n", argv[i]);
            }
        }
        if(opt_keepalive.op_equal(&opt_keepalive, &arg))
        {
            i++;
            struct String keepalive;
            String_ctorHold(&keepalive, argv[i], strlen(argv[i]));
            config->keepAlive = keepalive.toInt(&keepalive);
            if(config->keepAlive < 0)
            {
                fprintf(stderr, "Error parsing --keepalive string '%s'\n", argv[i]);
            }
        }
    }
    if(!logfile)
    {
        fprintf(stderr, "No log file provided (--log)\n");
        return 1;
    }
    tail(logfile, &tellme);
    struct Battle* battle = battleInstance();
    battle->dtor(battle);
    return 0;
}
