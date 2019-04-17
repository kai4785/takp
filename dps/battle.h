#ifndef BATTLE_H
#define BATTLE_H

#include "array.h"
#include "action.h"

#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>

struct Damage
{
    int64_t hits;
    int64_t damage;
};

double dps(struct Damage damage, int64_t seconds);
double hps(struct Damage damage, int64_t seconds);
double dph(struct Damage damage);

struct Fight;
void Fight_ctor(struct Fight* this);
struct Fight* Fight_new();
struct Fight
{
    int64_t sourceId;
    int64_t targetId;
    int64_t start;
    int64_t end;
    struct Damage m_melee;
    struct Damage m_backstab;
    struct Damage m_crit;
    struct Damage m_crip;
    struct Damage m_magic;
    int64_t (*seconds)(struct Fight* this);
    void (*dtor)(struct Fight* this);
};

struct Death
{
    int64_t sourceId;
    int64_t targetId;
    bool finishingBlow;
};

struct Battle;
void Battle_ctor(struct Battle* this);
struct Battle* Battle_new();
struct Battle
{
    struct Array m_pc;
    struct Array m_fight;
    struct Array m_death;
    int64_t m_start;
    int64_t m_end;
    int64_t m_expire;
    int64_t m_totalHeals;
    int64_t m_lastCrit;
    int64_t m_lastCrip;
    int64_t m_lastFinishingBlow;
    void (*start)(struct Battle* this, int64_t now);
    void (*reset)(struct Battle* this);
    bool (*inProgress)(struct Battle* this);
    bool (*isOver)(struct Battle* this, int64_t now);
    int64_t (*seconds)(struct Battle* this);
    void (*report)(struct Battle* this);
    void (*melee)(struct Battle* this, int64_t now, struct Action* action);
    void (*crit)(struct Battle* this, int64_t now, struct Action* action);
    void (*crip)(struct Battle* this, int64_t now, struct Action* action);
    void (*magic)(struct Battle* this, int64_t now, struct Action* action);
    void (*heal)(struct Battle* this, int64_t now, struct Action* action);
    void (*death)(struct Battle* this, int64_t now, struct Action* action);
    void (*dtor)(struct Battle* this);
};

struct Battle* battleInstance();

#endif // BATTLE_H
