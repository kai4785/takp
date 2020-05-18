#ifndef BATTLE_H
#define BATTLE_H

#include "action.h"

#include <string_view>
#include <cstdint>
#include <vector>

struct Damage
{
    int64_t hits;
    int64_t damage;
};

double dps(struct Damage damage, int64_t seconds);
double hps(struct Damage damage, int64_t seconds);
double dph(struct Damage damage);

struct Fight
{
    Fight(int64_t sourceId, int64_t m_targetId, int64_t start);
    int64_t m_sourceId;
    int64_t m_targetId;
    int64_t m_start;
    int64_t m_end;
    struct Damage m_melee;
    struct Damage m_backstab;
    struct Damage m_crit;
    struct Damage m_crip;
    struct Damage m_holyBlade;
    struct Damage m_magic;
    int64_t seconds();
};

struct Death
{
    int64_t m_sourceId;
    int64_t m_targetId;
    bool m_finishingBlow;
};

struct Battle
{
    Battle();
    std::vector<std::string> m_pc;
    std::vector<Fight> m_fight;
    std::vector<Death> m_death;
    int64_t m_start;
    int64_t m_end;
    int64_t m_expire;
    int64_t m_totalHeals;
    int64_t m_lastCrit;
    int64_t m_lastCrip;
    int64_t m_lastHolyBlade;
    int64_t m_lastFinishingBlow;
    void start(int64_t now);
    void reset();
    bool inProgress();
    bool isOver(int64_t now);
    int64_t seconds();
    void report();
    void melee(int64_t now, const Action& action);
    void crit(int64_t now, const Action& action);
    void crip(int64_t now, const Action& action);
    void holyBlade(int64_t now, const Action& action);
    void magic(int64_t now, const Action& action);
    void heal(int64_t now, const Action& action);
    void death(int64_t now, const Action& action);
private:
    bool isMe(const std::string_view& pc);
    int64_t getPCIndex(const std::string_view& pc);
    Fight& getFight(int64_t now, int64_t sourceId, int64_t targetId);
    void keepalive(int64_t now);
};

Battle* battleInstance();

#endif // BATTLE_H