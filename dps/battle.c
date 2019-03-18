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
void Battle_melee(struct Battle* this, struct Action* action);

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
    this->melee = &Battle_melee;
    this->dtor = &Battle_dtor;
    Array_ctor(&this->m_pc, sizeof(struct String));
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

void Battle_clearPCs(struct Battle *this)
{
    // Clear out all of the Strings
    for(size_t i = 0; i < battle.m_pc.size; i++)
    {
        struct String pc = *(struct String*)battle.m_pc.at(&battle.m_pc, i);
        pc.dtor(&pc);
    }
    battle.m_pc.clear(&battle.m_pc);
}

void Battle_dtor(struct Battle *this)
{
    // Clear out all of the Strings
    for(size_t i = 0; i < battle.m_pc.size; i++)
    {
        struct String pc = *(struct String*)battle.m_pc.at(&battle.m_pc, i);
        pc.dtor(&pc);
    }
    this->m_pc.dtor(&this->m_pc);
}

void Battle_start(struct Battle* this, int64_t now)
{
    this->m_start = now;
    this->m_end = now + config.keepAlive;
}

void Battle_reset(struct Battle* this)
{
    this->m_start = 0;
    this->m_end = 0;
    this->m_totalDamage = 0;
    this->m_totalHeals = 0;
    Battle_dtor(this);
    Array_ctor(&this->m_pc, sizeof(struct String));
}

void Battle_report(struct Battle* this)
{
    printf("Battle report! %ld -> %ld = %lds\n", this->m_start, this->m_end, this->m_end - this->m_start);
    printf("Total Damage: %ld\n", this->m_totalDamage);
    printf("Total Heals: %ld\n", this->m_totalHeals);
}

void Battle_melee(struct Battle* this, struct Action* action)
{
    struct String* foundSource = NULL;
    struct String* foundTarget = NULL;
    for(size_t i = 0; i < this->m_pc.size; i++)
    {
        struct String* tmp = (struct String*)this->m_pc.at(&this->m_pc, i);
        if(tmp)
        {
            if(&action->source.length && tmp->op_equal(tmp, &action->source))
                foundSource = tmp;
            if(&action->target.length && tmp->op_equal(tmp, &action->target))
                foundTarget = tmp;
            if(foundSource && foundTarget)
                break;
        }
    }
    if(!foundSource)
    {
        struct String saveme;
        String_ctorCopy(&saveme, &action->source);
        this->m_pc.push(&this->m_pc, &saveme);
    }
    if(!foundTarget)
    {
        struct String saveme;
        String_ctorCopy(&saveme, &action->target);
        this->m_pc.push(&this->m_pc, &saveme);
    }
    this->m_totalDamage += action->damage;
}
