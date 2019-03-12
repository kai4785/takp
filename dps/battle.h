#ifndef BATTLE_H
#define BATTLE_H

#include <stdbool.h>
#include <inttypes.h>

struct Battle
{
    int64_t totalDamage;
    int64_t totalHeals;
};

extern struct Battle battle;

void reportBattle();

#endif // BATTLE_H
