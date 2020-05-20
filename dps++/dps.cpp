#include "action.h"
#include "battle.h"
#include "config.h"
#include "date.h"
#include "tail.h"

#include <iostream>
#include <string_view>

using namespace std;
using namespace std::literals;

bool tellme(const std::string_view& line)
{
    auto& config = configInstance();
    static int64_t lineno = 0;
    lineno++;
    // empty line
    if(line.size() == 0)
        return true;;
    int64_t dateseconds = 0;

    if(line.size() < 27 || !(line[0] == '[' && line[25] == ']'))
    {
        cerr << "[" << lineno << "] No date in message(" << line.size() << "): " << line[0] << " " << line[25] << " [|" << line << "|]" << endl;
        return true;;
    }

    auto datestring = line.substr(5, 20);
    auto message = line.substr(27);
    dateseconds = parseDate(datestring);
    if(dateseconds < config.since)
    {
        return true;
    }
    if(config.until > 0 && dateseconds > config.until)
    {
        return false;
    }
    if(config.verbosity >= 9)
    {
        cout << "dateseconds: " << dateseconds << endl;
    }

    auto battle = battleInstance();
    if(battle->isOver(dateseconds))
    {
        battle->report();
        battle->reset();
    }

    Action action(message);
    switch(action.type())
    {
        case Action::CHAT:
        {
            cerr << message << endl;
            break;
        }
        case Action::MELEE:
        {
            battle->melee(dateseconds, action);
            break;
        }
        case Action::CRIT:
        {
            battle->crit(dateseconds, action);
            break;
        }
        case Action::CRIP:
        {
            battle->crip(dateseconds, action);
            break;
        }
        case Action::HOLYBLADE:
        {
            battle->holyBlade(dateseconds, action);
            break;
        }
        case Action::MAGIC:
        {
            battle->magic(dateseconds, action);
            break;
        }
        case Action::HEAL:
        {
            battle->heal(dateseconds, action);
            break;
        }
        case Action::DEATH:
        {
            battle->death(dateseconds, action);
            break;
        }
        case Action::UNKNOWN:
        default:
        {
            if(config.verbosity > 10)
                cerr << "[" << lineno << "]:[" << dateseconds << "]" << message << endl;
            break;
        }
    }
    return true;
}

void print_help()
{
    cout << "dps [--log <file>] [--me <name>] [--history] [--follow] [--since <date>] [--until <date>] [--keepalive <seconds>] [--verbosity <level>] [--help]" << endl;
    cout << "\t--log <file>            Filename of the log to parse" << endl;
    cout << "\t--me <name>             The name of the character that generated the logs" << endl;
    cout << "\t--follow                Parse the log file live as it is written to" << endl;
    cout << "\t--history               Parses the entire history of the log file." << endl;
    cout << "\t--since <date>          Skip to <date> in the log (see --history)" << endl;
    cout << "\t--until <date>          Stop at <date> in the log (see --history)" << endl;
    cout << "\t--keepalive <seconds>   The number of seconds between battles" << endl;
    cout << "\t--verbosity <level>     Pump up the verbosity" << endl;
    cout << "\t--help                  Print this help text" << endl;
    cout << "" << endl;
}

#define NEXT_ARG(_x) printf(" '%s'", *((_x)++))
int main(int argc, char **argv)
{
    auto& config = configInstance();
    auto opt_log = "--log"sv;
    auto opt_me = "--me"sv;
    auto opt_history = "--history"sv;
    auto opt_follow = "--follow"sv;
    auto opt_since = "--since"sv;
    auto opt_until = "--until"sv;
    auto opt_keepalive = "--keepalive"sv;
    auto opt_reportByTarget = "--by-target"sv;
    auto opt_help = "--help"sv;
    auto opt_verbosity = "--verbosity"sv;
    auto opt_asio = "--asio"sv;
    bool help = false;

    // Ew, manual parsing? Don't mess up, cause I'll just barf.
    string logfile;
    cout << "'" << argv[0] << "'";
    for(char** here = argv + 1; *here != NULL; NEXT_ARG(here))
    {
        string_view arg = *here;
        if(opt_help == arg)
        {
            help = true;
        }
        else if(opt_me == arg)
        {
            NEXT_ARG(here);
            config.me = *here;
        }
        else if(opt_log == arg)
        {
            NEXT_ARG(here);
            logfile = *here;
            auto eqlog_ = "eqlog_"sv;
            auto _loginse_txt = "_loginse.txt"sv;
            if(endsWith(logfile, _loginse_txt))
            {
                size_t pos = logfile.find(eqlog_);
                config.me = logfile.substr(eqlog_.size() + pos, logfile.size() - eqlog_.size() - pos - _loginse_txt.size());
            }
        }
        else if(opt_history == arg)
        {
            config.history = true;
        }
        else if(opt_follow == arg)
        {
            config.follow = true;
        }
        else if(opt_reportByTarget == arg)
        {
            config.reportByTarget = true;
        }
        else if(opt_since == arg)
        {
            NEXT_ARG(here);
            config.since = parseDate(*here);
            if(config.since < 0)
            {
                cerr << "Error parsing --since string '" << *here << "'" << endl;
            }
        }
        else if(opt_until == arg)
        {
            NEXT_ARG(here);
            string_view datestring = *here;
            config.until = parseDate(datestring);
            if(config.until < 0)
            {
                cerr << "Error parsing --until string '" << *here << "'" << endl;
            }
        }
        else if(opt_keepalive == arg)
        {
            NEXT_ARG(here);
            config.keepAlive = toInt(*here);
            if(config.keepAlive < 0)
            {
                cerr << "Error parsing --keepalive string '" << *here << "'" << endl;
            }
        }
        else if(opt_verbosity == arg)
        {
            NEXT_ARG(here);
            config.verbosity = toInt(*here);
            if(config.verbosity < 0)
            {
                cerr << "Error parsing --verbosity string '" << *here << "'" << endl;
            }
        }
        else if(opt_asio == arg)
        {
            config.asio = true;
        }
    }
    cout << endl;
    if(help)
    {
        print_help();
        return 0;
    }
    if(!logfile.size())
    {
        cerr << "No log file provided (--log)" << endl;
        print_help();
        return 1;
    }
    auto battle = battleInstance();
    tail(logfile, &tellme);
    if(battle->inProgress())
        battle->report();
    return 0;
}
