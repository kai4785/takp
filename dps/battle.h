#ifndef BATTLE_H
#define BATTLE_H

#include "array.h"

#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>

struct Fight;
void Fight_ctor(struct Fight* this);
struct Fight* Fight_new();
struct Fight
{
    int64_t start;
    int64_t end;
    void (*dtor)(struct Fight* this);
};

struct Battle;
void Battle_ctor(struct Battle* this);
struct Battle* Battle_new();
struct Battle
{
#if 0
    .melee = defaultdict(dict)
    .deaths = defaultdict(dict)
    .cripps = defaultdict(int)
    .crits = defaultdict(int)
#endif
    struct Array m_pc;
    int64_t m_start;
    int64_t m_end;
    int64_t m_totalDamage;
    int64_t m_totalHeals;
    void (*start)(struct Battle* this, int64_t now);
    void (*reset)(struct Battle* this);
    void (*report)(struct Battle* this);
    void (*dtor)(struct Battle* this);
};

extern struct Battle battle;

void startBattle();
void resetBattle();
void reportBattle();

#endif // BATTLE_H
