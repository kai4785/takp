#include "battle.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

// Glocal Battle object
struct Battle battle;

// Member function declarations
// Fight
void Fight_dtor(struct Fight *this);
// Battle
void Battle_dtor(struct Battle *this);
void Battle_reset(struct Battle *this);
void Battle_start(struct Battle* this, int64_t now);
void Battle_report(struct Battle* this);

// Constructors
void Fight_ctor(struct Fight* this)
{
    *this = (struct Fight){0};
    this->dtor = &Fight_dtor;
}

struct Fight* Fight_new()
{
    struct Fight* this = (struct Fight*)malloc(sizeof(struct Fight));
    Fight_ctor(this);
    return this;
}

void Battle_ctor(struct Battle* this)
{
    *this = (struct Battle){0};
    this->start = &Battle_start;
    this->reset = &Battle_reset;
    this->report = &Battle_report;
    this->dtor = &Battle_dtor;
}

struct Battle* Battle_new()
{
    struct Battle* this = (struct Battle*)malloc(sizeof(struct Battle));
    Battle_ctor(this);
    return this;
}

// Member function implementations
void Fight_dtor(struct Fight *this)
{
    (void)this;
}

void Battle_dtor(struct Battle *this)
{
    (void)this;
}

void Battle_start(struct Battle* this, int64_t now)
{
    this->m_start = now;
    this->m_end = now + config.keepAlive;
}

void Battle_reset(struct Battle* this)
{
    Battle_dtor(this);
    Battle_ctor(this);
}

void Battle_report(struct Battle* this)
{
    printf("Battle report! %ld -> %ld = %lds\n", this->m_start, this->m_end, this->m_end - this->m_start);
    printf("Total Damage: %ld\n", this->m_totalDamage);
    printf("Total Heals: %ld\n", this->m_totalHeals);
}
