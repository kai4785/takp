#include "battle.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

// Glocal Battle object
struct Battle* battleInstance()
{
    static struct Battle battle;
    static bool initialized = false;
    if(!initialized)
    {
        Battle_ctor(&battle);
        initialized = true;
    }
    return &battle;
}

// Member function declarations
// Fight
void Fight_dtor(struct Fight *this);
// Battle
void Battle_dtor(struct Battle *this);
void Battle_reset(struct Battle *this);
void Battle_start(struct Battle* this, int64_t now);
void Battle_report(struct Battle* this);
void Battle_melee(struct Battle* this, int64_t now, struct Action* action);

// Constructors
void Fight_ctor(struct Fight* this)
{
    *this = (struct Fight){
        .sourceId = 0,
        .targetId = 0,
        .start = 0,
        .end = 0,
        .hits = 0,
        .damage = 0,
        .dtor = &Fight_dtor
    };
}

struct Fight* Fight_new()
{
    struct Fight* this = (struct Fight*)malloc(sizeof(struct Fight));
    Fight_ctor(this);
    return this;
}

void Battle_ctor(struct Battle* this)
{
    *this = (struct Battle) {
        .m_start = 0,
        .m_end = 0,
        .m_totalDamage = 0,
        .m_totalHeals = 0,
        .start = &Battle_start,
        .reset = &Battle_reset,
        .report = &Battle_report,
        .melee = &Battle_melee,
        .dtor = &Battle_dtor
    };
    Array_ctor(&this->m_pc, sizeof(struct String));
    Array_ctor(&this->m_fight, sizeof(struct Fight));
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
    for(size_t i = 0; i < this->m_pc.size; i++)
    {
        struct String pc = *(struct String*)this->m_pc.at(&this->m_pc, i);
        pc.dtor(&pc);
    }
    this->m_pc.clear(&this->m_pc);
}

void Battle_dtor(struct Battle *this)
{
    // Clear out all of the Strings
    Battle_clearPCs(this);
    this->m_pc.dtor(&this->m_pc);
    this->m_fight.dtor(&this->m_fight);
}

void Battle_start(struct Battle* this, int64_t now)
{
    this->m_start = now;
    this->m_end = now + configInstance()->keepAlive;
}

void Battle_reset(struct Battle* this)
{
    this->m_start = 0;
    this->m_end = 0;
    this->m_totalDamage = 0;
    this->m_totalHeals = 0;
    Battle_clearPCs(this);
    this->m_fight.clear(&this->m_fight);
}

void Battle_report(struct Battle* this)
{
    printf("Battle report! %"PRId64" -> %"PRId64" = %"PRId64"s\n", this->m_start, this->m_end, this->m_end - this->m_start);
    printf("Total Heals: %"PRId64"\n", this->m_totalHeals);
    for(size_t i = 0; i < this->m_fight.size; i++)
    {
        struct Fight* fight = this->m_fight.at(&this->m_fight, i);
        struct String* source = this->m_pc.at(&this->m_pc, fight->sourceId);
        struct String* target = this->m_pc.at(&this->m_pc, fight->targetId);
        printf("Fight: %.*s -> %.*s, hits %"PRId64", damage %"PRId64", %"PRId64"s\n",
            (int)source->length, source->data,
            (int)target->length, target->data,
            fight->hits,
            fight->damage,
            fight->end - fight->start
        );
    }
}

bool Battle_isMe(struct String* pc)
{
    if(pc->length == 3)
    {
        if(pc->data[0] == 'Y')
        {
            if(
                (pc->data[1] == 'o' && pc->data[2] == 'u') ||
                (pc->data[1] == 'O' && pc->data[2] == 'U')
            )
            {
                return true;
            }
        }
    }
    return false;
}

int64_t Battle_getPCIndex(struct Battle* this, struct String* pc)
{
    int64_t id = -1;
    if(!pc || !pc->length)
        return id;
    struct SimpleString* findme = NULL;
    if(Battle_isMe(pc))
        findme = &(configInstance()->me);
    else
        findme = pc->to_SimpleString(pc);
    for(size_t i = 0; i < this->m_pc.size; i++)
    {
        struct String* tmp = (struct String*)this->m_pc.at(&this->m_pc, i);
        if(tmp && tmp->op_equal(tmp, findme))
        {
            id = i;
            break;
        }
    }
    if(id < 0)
    {
        struct String saveme;
        String_ctorCopy(&saveme, findme);
        id = this->m_pc.size;
        this->m_pc.push(&this->m_pc, &saveme);
        // Don't destroy saveme, we stuffed it into the array
    }
    return id;
}

struct Fight* Battle_getFightIndex(struct Battle* this, int64_t now, int64_t sourceId, int64_t targetId)
{
    for(size_t i = 0; i < this->m_fight.size; i++)
    {
        struct Fight* tmp = (struct Fight*)this->m_fight.at(&this->m_fight, i);
        if(tmp->sourceId == sourceId && tmp->targetId == targetId)
        {
            return tmp;
        }
    }
    // Make a new fight
    struct Fight fight;
    Fight_ctor(&fight);
    fight.sourceId = sourceId;
    fight.targetId = targetId;
    fight.start = now;
    this->m_fight.push(&this->m_fight, &fight);
    return (struct Fight*)this->m_fight.at(&this->m_fight, this->m_fight.size - 1);
}

void Battle_melee(struct Battle* this, int64_t now, struct Action* action)
{
    ssize_t sourceId = Battle_getPCIndex(this, &action->source);
    ssize_t targetId = Battle_getPCIndex(this, &action->target);

    struct Fight* fight = Battle_getFightIndex(this, now, sourceId, targetId);
    fight->hits++;
    fight->damage += action->damage;
    fight->end = now;

    this->m_totalDamage += action->damage;
}
