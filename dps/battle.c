#include "battle.h"

#include <stdio.h>

struct Battle battle;

void reportBattle()
{
    printf("Battle report!\n");
    printf("Total Damage: %ld\n", battle.totalDamage);
    printf("Total Heals: %ld\n", battle.totalHeals);
}
