#include "action.h"
#include "battle.h"
#include "config.h"
#include "date.h"
#include "tail.h"

#include <string.h>
#include <stdio.h>


bool tellme(struct SimpleString line)
{
    struct Config* config = configInstance();
    static int64_t lineno = 0;
    lineno++;
    // empty line
    if(line.length == 0)
        return true;;
    int64_t dateseconds = 0;

    if(line.length < 27 || !(line.data[0] == '[' && line.data[25] == ']'))
    {
        fprintf(stderr, "[%"PRId64"] No date in message(%zu): (%02x %02x) [|%s|]\n", lineno, line.length, line.data[0], line.data[25], line.data);
        return true;;
    }
    struct SimpleString datestring = {
        .data = line.data + 5,
        .length = 20
    };
    struct String message;
    String_ctorHold(&message,
        &line.data[27],
        line.length - 27
    );
    dateseconds = parseDate(datestring);
    if(dateseconds < config->since)
    {
        return true;
    }
    if(config->until > 0 && dateseconds > config->until)
    {
        return false;
    }
    if(config->verbosity > 10)
    {
        printf("dateseconds: %"PRId64"\n", dateseconds);
    }

    struct Battle* battle = battleInstance();
    if(battle->isOver(battle, dateseconds))
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
            fprintf(stderr, "%.*s\n", (int)message.length, message.data);
            break;
        }
        case MELEE:
        {
            battle->melee(battle, dateseconds, &action);
            break;
        }
        case CRIT:
        {
            battle->crit(battle, dateseconds, &action);
            break;
        }
        case CRIP:
        {
            battle->crip(battle, dateseconds, &action);
            break;
        }
        case HOLYBLADE:
        {
            battle->holyBlade(battle, dateseconds, &action);
            break;
        }
        case MAGIC:
        {
            battle->magic(battle, dateseconds, &action);
            break;
        }
        case HEAL:
        {
            battle->heal(battle, dateseconds, &action);
            break;
        }
        case DEATH:
        {
            battle->death(battle, dateseconds, &action);
            break;
        }
        case UNKNOWN:
        default:
        {
            if(config->verbosity > 10)
                fprintf(stderr, "[%"PRId64"]:[%"PRId64"]%.*s\n", lineno, dateseconds, (int)message.length, message.data);
            break;
        }
    }
    action.dtor(&action);
    return true;
}

void print_help()
{
    printf("dps [--log <file>] [--me <name>] [--history] [--follow] [--since <date>] [--until <date>] [--keepalive <seconds>] [--verbosity <level>] [--help]\n");
    printf("\t--log <file>            Filename of the log to parse\n");
    printf("\t--me <name>             The name of the character that generated the logs\n");
    printf("\t--follow                Parse the log file live as it is written to\n");
    printf("\t--history               Parses the entire history of the log file.\n");
    printf("\t--since <date>          Skip to <date> in the log (see --history)\n");
    printf("\t--until <date>          Stop at <date> in the log (see --history)\n");
    printf("\t--keepalive <seconds>   The number of seconds between battles\n");
    printf("\t--verbosity <level>     Pump up the verbosity\n");
    printf("\t--help                  Print this help text\n");
    printf("\n");
}

int main(int argc, char **argv)
{
    struct Config* config = configInstance();
    struct String opt_log = CONST_STRING("--log");
    struct String opt_me = CONST_STRING("--me");
    struct String opt_history = CONST_STRING("--history");
    struct String opt_follow = CONST_STRING("--follow");
    struct String opt_since = CONST_STRING("--since");
    struct String opt_until = CONST_STRING("--until");
    struct String opt_keepalive = CONST_STRING("--keepalive");
    struct String opt_reportByTarget = CONST_STRING("--by-target");
    struct String opt_help = CONST_STRING("--help");
    struct String opt_verbosity = CONST_STRING("--verbosity");
    bool help = false;

    // Ew, manual parsing? Don't mess up, cause I'll just barf.
    char* logfile = NULL;
    for(int i = 1; i < argc; i++)
    {
        struct SimpleString arg = SIMPLE_STRING(argv[i]);
        if(opt_help.op_equal(&opt_help, &arg))
        {
            help = true;
        }
        else if(opt_me.op_equal(&opt_me, &arg))
        {
            i++;
            config->me = (struct SimpleString)SIMPLE_STRING(argv[i]);
        }
        else if(opt_log.op_equal(&opt_log, &arg))
        {
            i++;
            logfile = argv[i];
        }
        else if(opt_history.op_equal(&opt_history, &arg))
        {
            config->history = true;
        }
        else if(opt_follow.op_equal(&opt_follow, &arg))
        {
            config->follow = true;
        }
        else if(opt_follow.op_equal(&opt_reportByTarget, &arg))
        {
            config->reportByTarget = true;
        }
        else if(opt_since.op_equal(&opt_since, &arg))
        {
            i++;
            struct SimpleString datestring = (struct SimpleString)SIMPLE_STRING(argv[i]);
            config->since = parseDate(datestring);
            if(config->since < 0)
            {
                fprintf(stderr, "Error parsing --since string '%s'\n", argv[i]);
            }
        }
        else if(opt_until.op_equal(&opt_until, &arg))
        {
            i++;
            struct SimpleString datestring = (struct SimpleString)SIMPLE_STRING(argv[i]);
            config->until = parseDate(datestring);
            if(config->until < 0)
            {
                fprintf(stderr, "Error parsing --until string '%s'\n", argv[i]);
            }
        }
        else if(opt_keepalive.op_equal(&opt_keepalive, &arg))
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
        else if(opt_verbosity.op_equal(&opt_verbosity, &arg))
        {
            i++;
            struct String verbosity;
            String_ctorHold(&verbosity, argv[i], strlen(argv[i]));
            config->verbosity = (int)verbosity.toInt(&verbosity);
            if(config->verbosity < 0)
            {
                fprintf(stderr, "Error parsing --verbosity string '%s'\n", argv[i]);
            }
        }
    }
    if(help)
    {
        print_help();
        return 0;
    }
    if(!logfile)
    {
        fprintf(stderr, "No log file provided (--log)\n");
        print_help();
        return 1;
    }
    struct Battle* battle = battleInstance();
    tail(logfile, &tellme);
    if(battle->inProgress(battle))
        battle->report(battle);
    battle->dtor(battle);
    return 0;
}
