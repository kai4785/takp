#include "battle.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

// Glocal Battle object
struct Battle* battleInstance()
{
    static struct Battle battle = {0};
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
int64_t Fight_seconds(struct Fight *this);
double Fight_dps(struct Fight *this);
double Fight_hps(struct Fight *this);
double Fight_dph(struct Fight *this);
// Battle
void Battle_dtor(struct Battle *this);
void Battle_reset(struct Battle *this);
void Battle_start(struct Battle* this, int64_t now);
void Battle_report(struct Battle* this);
void Battle_melee(struct Battle* this, int64_t now, struct Action* action);
void Battle_magic(struct Battle* this, int64_t now, struct Action* action);

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
        .seconds = &Fight_seconds,
        .dps = &Fight_dps,
        .hps = &Fight_hps,
        .dph = &Fight_dph,
        .dtor = &Fight_dtor
    };
}

struct Fight* Fight_new()
{
    struct Fight* this = (struct Fight*)malloc(sizeof(struct Fight));
    Fight_ctor(this);
    return this;
}

int64_t Fight_seconds(struct Fight* this)
{
    if(this->end == this->start)
        return 1;
    return this->end - this->start;
}

double Fight_dps(struct Fight* this)
{
    return (double)(this->damage) / (double)(this->seconds(this));
}

double Fight_hps(struct Fight* this)
{
    return (double)(this->hits) / (double)(this->seconds(this));
}

double Fight_dph(struct Fight* this)
{
    return (double)(this->damage) / (double)(this->hits);
}

void Battle_String_dtor(void* string)
{
    struct String* deleteme = (struct String*) string;
    deleteme->dtor(deleteme);
}

void Battle_ctor(struct Battle* this)
{
    *this = (struct Battle) {
        .m_start = 0,
        .m_end = 0,
        .m_expire = 0,
        .m_totalDamage = 0,
        .m_totalHeals = 0,
        .start = &Battle_start,
        .reset = &Battle_reset,
        .report = &Battle_report,
        .melee = &Battle_melee,
        .magic = &Battle_magic,
        .dtor = &Battle_dtor
    };
    Array_ctor(&this->m_pc, sizeof(struct String));
    Array_ctor(&this->m_fight, sizeof(struct Fight));
    this->m_pc.datum_dtor = Battle_String_dtor;
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
    // Clear out all of the Strings
    this->m_pc.dtor(&this->m_pc);
    this->m_fight.dtor(&this->m_fight);
}

void Battle_start(struct Battle* this, int64_t now)
{
    this->m_start = now;
    this->m_end = now;
    this->m_expire = now + configInstance()->keepAlive;
}

void Battle_reset(struct Battle* this)
{
    this->m_start = 0;
    this->m_end = 0;
    this->m_expire = 0;
    this->m_totalDamage = 0;
    this->m_totalHeals = 0;
    this->m_pc.clear(&this->m_pc);
    this->m_fight.clear(&this->m_fight);
}

void Battle_report(struct Battle* this)
{
    int64_t battleSeconds = this->m_end - this->m_start;
    if(battleSeconds == 0)
        battleSeconds = 1;
    printf("Battle report! %"PRId64"s [%"PRId64" : %"PRId64"]\n", battleSeconds, this->m_start, this->m_end);
    #define break_str "-------------------------------------------------------------------------------------------------------"
    #define header_format "%-35s %-30s %4s %4s %5s %6s %6s %6s\n"
    #define fight_format "%-35.*s %-30.*s %4"PRId64" %4"PRId64" %5.2f %6"PRId64" %6.2f %6.2f\n"
    printf(header_format, "(N)PC", "Target", "Sec", "Hits", "h/s", "Damage", "d/h", "d/s");
    printf("%s\n", break_str);
    for(int64_t pcId = 0; pcId < this->m_pc.size; pcId++)
    {
        int targets = 0;
        int64_t totalHits = 0;
        int64_t totalDamage = 0;
        for(size_t i = 0; i < this->m_fight.size; i++)
        {
            struct Fight* fight = this->m_fight.at(&this->m_fight, i);
            if(fight->sourceId != pcId)
                continue;
            targets++;
            struct String* source = this->m_pc.at(&this->m_pc, fight->sourceId);
            struct String* target = this->m_pc.at(&this->m_pc, fight->targetId);
            printf(fight_format,
                (int)source->length, (targets == 1) ? source->data : "",
                (int)target->length, target->data,
                fight->seconds(fight),
                fight->hits,
                fight->hps(fight),
                fight->damage,
                fight->dph(fight),
                fight->dps(fight)
            );
            totalHits += fight->hits;
            totalDamage += fight->damage;
        }
        if(targets > 1)
            printf(fight_format,
                5, "Total",
                0, "",
                battleSeconds,
                totalHits,
                (double)totalHits / (double)battleSeconds,
                totalDamage,
                (double)totalDamage / (double)totalHits,
                (double)totalDamage / (double)battleSeconds
            );
        if(targets > 0)
            printf("%s\n", break_str);
    }
    printf("Total Heals: %"PRId64"\n", this->m_totalHeals);
    printf("\n");
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
    struct Fight fight = {0};
    Fight_ctor(&fight);
    fight.sourceId = sourceId;
    fight.targetId = targetId;
    fight.start = now;
    this->m_fight.push(&this->m_fight, &fight);
    return (struct Fight*)this->m_fight.at(&this->m_fight, this->m_fight.size - 1);
}

void Battle_melee(struct Battle* this, int64_t now, struct Action* action)
{
    int64_t sourceId = Battle_getPCIndex(this, &action->source);
    int64_t targetId = Battle_getPCIndex(this, &action->target);

    struct Fight* fight = Battle_getFightIndex(this, now, sourceId, targetId);
    fight->hits++;
    fight->damage += action->damage;
    fight->end = now;

    this->m_totalDamage += action->damage;
    this->m_end = now;
    this->m_expire = now + configInstance()->keepAlive;
}

void Battle_magic(struct Battle* this, int64_t now, struct Action* action)
{
    Battle_melee(this, now, action);
}
