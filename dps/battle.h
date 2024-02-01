#ifndef BATTLE_H
#define BATTLE_H

#include "array.h"
#include "action.h"

#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>

struct MeleeDamage;
void MeleeDamage_ctor(struct MeleeDamage* this);
struct MeleeDamage
{
    int64_t m_hits;
    int64_t m_damage;
    int64_t m_crit_hits;
    int64_t m_crit_damage;
    int64_t m_crip_hits;
    int64_t m_crip_damage;
    int64_t m_holy_hits;
    int64_t m_holy_damage;
    int64_t m_misses;
    int64_t m_parries;
    int64_t m_dodges;
    int64_t m_ripostes;
    int64_t m_blocks;
    int64_t m_absorb;
    int64_t (*swings)(struct MeleeDamage* this);
    int64_t (*misses)(struct MeleeDamage* this);
    int64_t (*strikes)(struct MeleeDamage* this);
    int64_t (*avoided)(struct MeleeDamage* this);
    int64_t (*hits)(struct MeleeDamage* this);
    int64_t (*damage)(struct MeleeDamage* this);
};

struct MagicDamage
{
    int64_t hits;
    int64_t damage;
    int64_t crit_hits;
    int64_t crit_damage;
};

struct Fight;
void Fight_ctor(struct Fight* this);
struct Fight* Fight_new();
struct Fight
{
    int64_t sourceId;
    int64_t targetId;
    int64_t start;
    int64_t end;
    struct MeleeDamage m_melee;
    struct MagicDamage m_magic;
    struct Array m_byVerb;
    struct MeleeDamage* (*getMeleeDamage)(struct Fight* this, struct SimpleString* verb);
    int64_t (*seconds)(struct Fight* this);
    int64_t (*swings)(struct Fight* this);
    int64_t (*misses)(struct Fight* this);
    int64_t (*strikes)(struct Fight* this);
    int64_t (*avoided)(struct Fight* this);
    int64_t (*hits)(struct Fight* this);
    int64_t (*damage)(struct Fight* this);
    void (*dtor)(void* this);
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
    int64_t m_lastHolyBlade;
    int64_t m_lastFinishingBlow;
    void (*start)(struct Battle* this, int64_t now);
    void (*reset)(struct Battle* this);
    bool (*inProgress)(struct Battle* this);
    bool (*isOver)(struct Battle* this, int64_t now);
    int64_t (*seconds)(struct Battle* this);
    void (*report)(struct Battle* this);
    void (*melee)(struct Battle* this, int64_t now, struct Action* action);
    void (*miss)(struct Battle* this, int64_t now, struct Action* action);
    void (*crit)(struct Battle* this, int64_t now, struct Action* action);
    void (*crip)(struct Battle* this, int64_t now, struct Action* action);
    void (*holyBlade)(struct Battle* this, int64_t now, struct Action* action);
    void (*magic)(struct Battle* this, int64_t now, struct Action* action);
    void (*heal)(struct Battle* this, int64_t now, struct Action* action);
    void (*death)(struct Battle* this, int64_t now, struct Action* action);
    void (*sort)(struct Battle* this);
    void (*dtor)(struct Battle* this);
};

struct Battle* battleInstance();

#endif // BATTLE_H
