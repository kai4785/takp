#ifndef BATTLE_H
#define BATTLE_H

#include "array.h"
#include "action.h"

#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>

struct Fight;
void Fight_ctor(struct Fight* this);
struct Fight* Fight_new();
struct Fight
{
    int64_t sourceId;
    int64_t targetId;
    int64_t start;
    int64_t end;
    int64_t hits;
    int64_t damage;
    int64_t (*seconds)(struct Fight* this);
    double (*dps)(struct Fight* this);
    double (*hps)(struct Fight* this);
    double (*dph)(struct Fight* this);
    void (*dtor)(struct Fight* this);
};

struct Battle;
void Battle_ctor(struct Battle* this);
struct Battle* Battle_new();
struct Battle
{
#if 0
    .deaths = defaultdict(dict)
    .cripps = defaultdict(int)
    .crits = defaultdict(int)
#endif
    struct Array m_pc;
    struct Array m_fight;
    int64_t m_start;
    int64_t m_end;
    int64_t m_expire;
    int64_t m_totalDamage;
    int64_t m_totalHeals;
    void (*start)(struct Battle* this, int64_t now);
    void (*reset)(struct Battle* this);
    void (*report)(struct Battle* this);
    void (*melee)(struct Battle* this, int64_t now, struct Action* action);
    void (*magic)(struct Battle* this, int64_t now, struct Action* action);
    void (*dtor)(struct Battle* this);
};

struct Battle* battleInstance();

#endif // BATTLE_H
