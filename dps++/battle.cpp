#include "battle.h"
#include "date.h"
#include "utility.h"
#include "config.h"

#include <algorithm>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace std::literals::string_view_literals;

// Glocal Battle object
struct Battle* battleInstance()
{
    static Battle battle;
    return &battle;
}

// Helper Functions
double dps(struct Damage damage, int64_t seconds)
{
    return (double)(damage.damage) / (double)(seconds);
}

double hps(struct Damage damage, int64_t seconds)
{
    return (double)(damage.hits) / (double)(seconds);
}

double dph(struct Damage damage)
{
    if(damage.hits)
        return (double)(damage.damage) / (double)(damage.hits);
    else
        return 0;
}

// Constructors
Fight::Fight(int64_t sourceId, int64_t targetId, int64_t start)
    :m_sourceId(sourceId)
    ,m_targetId(targetId)
    ,m_start(start)
    ,m_end(0)
    ,m_melee{0,0}
    ,m_backstab{0,0}
    ,m_crit{0,0}
    ,m_crip{0,0}
    ,m_holyBlade{0,0}
    ,m_magic{0,0}
{
}

int64_t Fight::seconds()
{
    if(m_end == m_start)
        return 1;
    return m_end - m_start;
}

Battle::Battle()
    :m_start(-1)
    ,m_end(-1)
    ,m_expire(0)
    ,m_totalHeals(0)
    ,m_lastCrit(0)
    ,m_lastCrip(0)
    ,m_lastHolyBlade(0)
    ,m_lastFinishingBlow(-1)
{
}

// Member function implementations
void Battle::start(int64_t now)
{
    if(!inProgress())
    {
        m_start = now;
        Battle::keepalive(now);
    }
}

void Battle::keepalive(int64_t now)
{
    m_end = now;
    m_expire = now + Config::instance().keepAlive;
}

bool Battle::isOver(int64_t now)
{
    return (m_start > -1 && m_expire < now);
}

bool Battle::inProgress()
{
    return seconds() > 0;
}

int64_t Battle::seconds()
{
    if(m_start == -1)
        return 0;
    if(m_end == m_start)
        return 1;
    return m_end - m_start;
}

void Battle::reset()
{
    m_start = -1;
    m_end = -1;
    m_expire = 0;
    m_totalHeals = 0;
    m_lastCrit = 0;
    m_lastCrip = 0;
    m_lastHolyBlade = 0;
    m_lastFinishingBlow = -1;
    m_pc.clear();
    m_fight.clear();
    m_death.clear();
}

void print_start(int64_t battleSeconds, const string_view& startDate, const string_view& endDate)
{
    cout << "===============  Battle report! "
         << setw(6) << battleSeconds
         << "s [" << startDate << " - " << endDate << "]"
         << "   ==============="
    << endl;

}

void print_break()
{
    cout << "-------------------------------------------------------------------------------------------------------" << endl;
}

void print_header()
{
    //#define header_format "%-35s %-30s %4s %4s %5s %6s %6s %6s\n"
    cout << left
         << setw(35) << "(N)PC" << " "
         << setw(30) << "Target" << " "
         << right
         << setw(4) << "Sec" << " "
         << setw(4) << "Hits" << " "
         << setw(5) << "h/s" << " "
         << setw(6) << "Damage" << " "
         << setw(6) << "d/h" << " "
         << setw(6) << "d/s" << endl;
}

void print_melee(const string_view& source, const string_view& target,
    int64_t seconds, int64_t hits, double hps, int64_t damage, double dph, double dps)
{
    //#define melee_format "%-35.*s %-30.*s %4"PRId64" %4"PRId64" %5.2f %6"PRId64" %6.2f %6.2f\n"
    cout << left << fixed << setprecision(2)
         << setw(35) << source << " "
         << setw(30) << target << " "
         << right
         << setw(4) << seconds << " "
         << setw(4) << hits << " "
         << setw(5) << hps << " "
         << setw(6) << damage << " "
         << setw(6) << dph << " "
         << setw(6) << dps << endl;
}

void print_special_damage(const string_view& type,
    int64_t hits, double hps, int64_t damage, double dph, double dps)
{
    //#define backstab_format "%-35s %-30s %4s %4"PRId64" %5.2f %6"PRId64" %6.2f %6.2f\n"
    //#define crit_format backstab_format
    //#define crip_format backstab_format
    //#define holyBlade_format backstab_format
    //#define magic_format backstab_format
    cout << left << fixed << setprecision(2)
         << setw(35) << "" << " "
         << setw(30) << type << " "
         << right
         << setw(4) << "" << " "
         << setw(4) << hits << " "
         << setw(5) << hps << " "
         << setw(6) << damage << " "
         << setw(6) << dph << " "
         << setw(6) << dps << endl;
}

void print_total(const string_view& description,
    int64_t seconds, int64_t hits, double hps, int64_t damage, double dph, double dps)
{
    //#define total_format "%-35s %-30s %4"PRId64" %4"PRId64" %5.2f %6"PRId64" %6.2f %6.2f\n"
    cout << left << fixed << setprecision(2)
         << setw(35) << "" << " "
         << setw(30) << description << " "
         << right
         << setw(4) << seconds << " "
         << setw(4) << hits << " "
         << setw(5) << hps << " "
         << setw(6) << damage << " "
         << setw(6) << dph << " "
         << setw(6) << dps << endl;
}

void print_heals(int64_t totalHeals)
{
    cout << "Total Heals: " << totalHeals << endl;
}

void Battle::report()
{
    // Report the date
    StringBuf startDateBuf(20);
    StringBuf endDateBuf(20);
    auto& config = Config::instance();
    int64_t battleSeconds = m_end - m_start;
    if(battleSeconds == 0)
        battleSeconds = 1;
    unparseDate(m_start, startDateBuf);
    string_view startDate = startDateBuf;
    unparseDate(m_end, endDateBuf);
    string_view endDate = endDateBuf;

    print_start(battleSeconds, startDate, endDate);
    print_header();
    print_break();
    for(int64_t pcId = 0; pcId < m_pc.size(); pcId++)
    {
        int linesPrinted = 0;
        int64_t totalHits = 0;
        int64_t totalDamage = 0;
        for(size_t fightId = 0; fightId < m_fight.size(); fightId++)
        {
            auto& fight = m_fight.at(fightId);
            int64_t id = config.reportByTarget ? fight.m_targetId : fight.m_sourceId;
            bool printSource = (!linesPrinted ||  config.reportByTarget);
            bool printTarget = (!linesPrinted || !config.reportByTarget);
            if(id != pcId)
                continue;
            // TODO: Handle Ids that are less than 0
            auto source = fight.m_sourceId >= 0 ? m_pc.at(fight.m_sourceId) : ""sv;
            auto target = fight.m_targetId >= 0 ? m_pc.at(fight.m_targetId) : ""sv;
            int64_t fightSeconds = fight.seconds();
            print_melee(
                printSource ? source : "",
                printTarget ? target : "",
                fightSeconds,
                fight.m_melee.hits,
                hps(fight.m_melee, fightSeconds),
                fight.m_melee.damage,
                dph(fight.m_melee),
                dps(fight.m_melee, fightSeconds)
            );
            totalHits += fight.m_melee.hits;
            totalDamage += fight.m_melee.damage;
            linesPrinted++;
            if(fight.m_backstab.hits > 0)
            {
                print_special_damage(
                    "  *backstabs",
                    fight.m_backstab.hits,
                    hps(fight.m_backstab, fightSeconds),
                    fight.m_backstab.damage,
                    dph(fight.m_backstab),
                    dps(fight.m_backstab, fightSeconds)
                );
                linesPrinted++;
            }
            if(fight.m_crit.hits > 0)
            {
                print_special_damage(
                    "  *critical hits",
                    fight.m_crit.hits,
                    hps(fight.m_crit, fightSeconds),
                    fight.m_crit.damage,
                    dph(fight.m_crit),
                    dps(fight.m_crit, fightSeconds)
                );
                linesPrinted++;
            }
            if(fight.m_crip.hits > 0)
            {
                print_special_damage(
                    "  *crippling blows",
                    fight.m_crip.hits,
                    hps(fight.m_crip, fightSeconds),
                    fight.m_crip.damage,
                    dph(fight.m_crip),
                    dps(fight.m_crip, fightSeconds)
                );
                linesPrinted++;
            }
            if(fight.m_holyBlade.hits > 0)
            {
                print_special_damage(
                    "  *holy blade",
                    fight.m_holyBlade.hits,
                    hps(fight.m_holyBlade, fightSeconds),
                    fight.m_holyBlade.damage,
                    dph(fight.m_holyBlade),
                    dps(fight.m_holyBlade, fightSeconds)
                );
                linesPrinted++;
            }
            if(fight.m_magic.hits > 0)
            {
                print_special_damage(
                    "  *spell/ds",
                    fight.m_magic.hits,
                    hps(fight.m_magic, fightSeconds),
                    fight.m_magic.damage,
                    dph(fight.m_magic),
                    dps(fight.m_magic, fightSeconds)
                );
                linesPrinted++;
            }
        }
        if(linesPrinted > 0)
        {
            print_total(
                "  *battle",
                battleSeconds,
                totalHits,
                (double)totalHits / (double)battleSeconds,
                totalDamage,
                totalHits ? (double)totalDamage / (double)totalHits : 0,
                (double)totalDamage / (double)battleSeconds
            );
            print_break();
        }
    }
    if(m_totalHeals > 0)
    {
        print_heals(m_totalHeals);
        print_break();
    }
    if(m_death.size() > 0)
    {
        cout << "Deaths: " << m_death.size() << endl;;
        // #define death_format "%-35.*s %-30.*s %s\n"
        cout << left
             << setw(35) << "Target" << " "
             << setw(30) << "Slayer" << " "
             << right << endl;
        print_break();
        for(auto& death : m_death)
        {
            auto source = (death.m_sourceId >= 0) ? m_pc.at(death.m_sourceId) : ""sv;
            auto target = (death.m_targetId >= 0) ? m_pc.at(death.m_targetId) : ""sv;
            auto finish = (death.m_finishingBlow) ? "Finishing Blow"sv : ""sv;
            cout << left
                 << setw(35) << target << " "
                 << setw(30) << source << " "
                 << right
                 << finish << endl;
        }
        print_break();
    }
    cout << endl;
}

bool Battle::isMe(const string_view& pc)
{
    if(pc.length() == 3)
    {
        if(pc[0] == 'Y')
        {
            if(
                (pc[1] == 'o' && pc[2] == 'u') ||
                (pc[1] == 'O' && pc[2] == 'U')
            )
            {
                return true;
            }
        }
    }
    return false;
}

int64_t Battle::getPCIndex(const string_view& pc)
{
    int64_t id = -1;
    if(pc.size() == 0)
        return -1;
    id = 0;

    string_view findme = (isMe(pc)) ? Config::instance().me : pc;

    for(id = 0; id < m_pc.size(); id++)
    {
        if(m_pc[id] == findme)
            return id;
    }
    m_pc.emplace_back(findme);
    return m_pc.size() - 1;
}

Fight& Battle::getFight(int64_t now, int64_t sourceId, int64_t targetId)
{
    for(auto& fight : m_fight)
    {
        if(fight.m_sourceId == sourceId && fight.m_targetId == targetId)
        {
            return fight;
        }
    }
    // Make a new fight
    m_fight.emplace_back(sourceId, targetId, now);
    return m_fight.back();
}

void Battle::melee(int64_t now, const Action& action)
{
    start(now);
    int64_t sourceId = getPCIndex(action.source());
    int64_t targetId = getPCIndex(action.target());
    // TODO: Handle finishing blows separately
    // Preceeded by a "<Name> scores a Finishing Blow!!"
    if(action.type() == Action::MELEE && action.damage() >= 32000)
    {
        m_lastFinishingBlow = targetId;
        return;
    }

    auto& fight = getFight(now, sourceId, targetId);
    fight.m_melee.hits++;
    fight.m_melee.damage += action.damage();
    fight.m_end = now;
    if(startsWith(action.verb(), "backstab"sv))
    {
        fight.m_backstab.hits++;
        fight.m_backstab.damage += action.damage();
    }
    else if(m_lastCrit == action.damage())
    {
        fight.m_crit.hits++;
        fight.m_crit.damage += action.damage();
        m_lastCrit = 0;
    }
    else if(m_lastCrip == action.damage())
    {
        fight.m_crip.hits++;
        fight.m_crip.damage += action.damage();
        m_lastCrip = 0;
    }
    else if(m_lastHolyBlade == action.damage())
    {
        fight.m_holyBlade.hits++;
        fight.m_holyBlade.damage += action.damage();
        m_lastHolyBlade = 0;
    }

    keepalive(now);
}

void Battle::crit(int64_t now, const Action& action)
{
    m_lastCrit = action.damage();
}

void Battle::crip(int64_t now, const Action& action)
{
    m_lastCrip = action.damage();
}

void Battle::holyBlade(int64_t now, const Action& action)
{
    m_lastHolyBlade = action.damage();
}

void Battle::magic(int64_t now, const Action& action)
{
    melee(now, action);

    start(now);
    int64_t sourceId = getPCIndex(action.source());
    int64_t targetId = getPCIndex(action.target());

    auto& fight = getFight(now, sourceId, targetId);
    fight.m_magic.hits++;
    fight.m_magic.damage += action.damage();
    fight.m_end = now;

    keepalive(now);
}

void Battle::heal(int64_t now, const Action& action)
{
    start(now);
    m_totalHeals += action.damage();
    keepalive(now);
}

void Battle::death(int64_t now, const Action& action)
{
    start(now);
    struct Death death = {0};
    death.m_sourceId = getPCIndex(action.source());
    death.m_targetId = getPCIndex(action.target());
    if(death.m_targetId == m_lastFinishingBlow)
    {
        death.m_finishingBlow = true;
        m_lastFinishingBlow = -1;
    }
    m_death.push_back(death);
    keepalive(now);
}
