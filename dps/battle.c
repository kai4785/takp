#include "battle.h"
#include "date.h"
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

struct ByVerb
{
    struct String m_verb;
    struct MeleeDamage m_melee;
};

void ByVerb_ctor(struct ByVerb *this, struct SimpleString* verb)
{
    String_ctorCopyString(&this->m_verb, verb);
    MeleeDamage_ctor(&this->m_melee);
}

void ByVerb_dtor(void *_this)
{
    struct ByVerb *this = (struct ByVerb *)_this;
    this->m_verb.dtor(&this->m_verb);
}

// Member function declarations
int64_t MeleeDamage_swings(struct MeleeDamage* this);
int64_t MeleeDamage_misses(struct MeleeDamage* this);
int64_t MeleeDamage_strikes(struct MeleeDamage* this);
int64_t MeleeDamage_avoided(struct MeleeDamage* this);
int64_t MeleeDamage_hits(struct MeleeDamage* this);
int64_t MeleeDamage_damage(struct MeleeDamage* this);
// Fight
struct MeleeDamage* Fight_getMeleeDamage(struct Fight* this, struct SimpleString* verb);
int64_t Fight_seconds(struct Fight *this);
int64_t Fight_swings(struct Fight* this);
int64_t Fight_misses(struct Fight* this);
int64_t Fight_strikes(struct Fight* this);
int64_t Fight_hits(struct Fight* this);
int64_t Fight_damage(struct Fight* this);
int64_t Fight_avoided(struct Fight* this);
void Fight_dtor(void* this);
// Battle
void Battle_reset(struct Battle *this);
void Battle_start(struct Battle* this, int64_t now);
void Battle_keepalive(struct Battle* this, int64_t now);
bool Battle_inProgress(struct Battle *this);
bool Battle_isOver(struct Battle *this, int64_t now);
int64_t Battle_seconds(struct Battle *this);
void Battle_report(struct Battle* this);
void Battle_melee(struct Battle* this, int64_t now, struct Action* action);
void Battle_miss(struct Battle* this, int64_t now, struct Action* action);
void Battle_crit(struct Battle* this, int64_t now, struct Action* action);
void Battle_crip(struct Battle* this, int64_t now, struct Action* action);
void Battle_holyBlade(struct Battle* this, int64_t now, struct Action* action);
void Battle_magic(struct Battle* this, int64_t now, struct Action* action);
void Battle_heal(struct Battle* this, int64_t now, struct Action* action);
void Battle_death(struct Battle* this, int64_t now, struct Action* action);
void Battle_sort(struct Battle* this);
void Battle_dtor(struct Battle *this);

// Constructors
void MeleeDamage_ctor(struct MeleeDamage* this)
{
    *this = (struct MeleeDamage){
        .m_hits = 0,
        .m_damage = 0,
        .m_crit_hits = 0,
        .m_crit_damage = 0,
        .m_crip_hits = 0,
        .m_crip_damage = 0,
        .m_holy_hits = 0,
        .m_holy_damage = 0,
        .m_misses = 0,
        .m_parries = 0,
        .m_dodges = 0,
        .m_ripostes = 0,
        .m_blocks = 0,
        .m_absorb = 0,
        .swings = &MeleeDamage_swings,
        .misses = &MeleeDamage_misses,
        .strikes = &MeleeDamage_strikes,
        .avoided = &MeleeDamage_avoided,
        .hits = &MeleeDamage_hits,
        .damage = &MeleeDamage_damage
    };
}

void Fight_ctor(struct Fight* this)
{
    *this = (struct Fight){
        .sourceId = 0,
        .targetId = 0,
        .start = 0,
        .end = 0,
        .m_magic = {0},
        .getMeleeDamage = &Fight_getMeleeDamage,
        .seconds = &Fight_seconds,
        .swings = &Fight_swings,
        .misses = &Fight_misses,
        .strikes = &Fight_strikes,
        .avoided = &Fight_avoided,
        .hits = &Fight_hits,
        .damage = &Fight_damage,
        .dtor = &Fight_dtor
    };
    Array_ctor(&this->m_byVerb, sizeof(struct ByVerb));
    this->m_byVerb.datum_dtor = ByVerb_dtor;
    MeleeDamage_ctor(&this->m_melee);
}

struct Fight* Fight_new()
{
    struct Fight* this = (struct Fight*)malloc(sizeof(struct Fight));
    Fight_ctor(this);
    return this;
}

struct MeleeDamage* Fight_getMeleeDamage(struct Fight* this, struct SimpleString *verb)
{
    struct Config* config = configInstance();
    if(config->byVerb)
    {
        struct ByVerb* byVerb = NULL;
        for(size_t i = 0; i < this->m_byVerb.size; i++)
        {
            struct ByVerb* _byVerb = (struct ByVerb*)this->m_byVerb.at(&this->m_byVerb, i);
            if(_byVerb && _byVerb->m_verb.op_equal(&_byVerb->m_verb, verb))
            {
                byVerb = _byVerb;
                break;
            }
        }
        if(!byVerb)
        {
            struct ByVerb _byVerb;
            ByVerb_ctor(&_byVerb, verb);
            this->m_byVerb.push(&this->m_byVerb, &_byVerb);
            byVerb = (struct ByVerb*)this->m_byVerb.at(&this->m_byVerb, this->m_byVerb.size - 1);
        }
        return &byVerb->m_melee;
    }
    else
    {
        return &this->m_melee;
    }
}

int64_t Fight_seconds(struct Fight* this)
{
    if(this->end == this->start)
        return 1;
    return this->end - this->start;
}

int64_t Fight_swings(struct Fight* this)
{
    return this->strikes(this) + this->misses(this);
}

int64_t Fight_misses(struct Fight* this)
{
    if(this->m_byVerb.size)
    {
        int64_t total = 0;
        for(size_t i = 0; i < this->m_byVerb.size; i++)
        {
            struct ByVerb* byVerb = (struct ByVerb*)this->m_byVerb.at(&this->m_byVerb, i);
            total += byVerb->m_melee.misses(&byVerb->m_melee);
        }
        return total;
    }
    return this->m_melee.misses(&this->m_melee);
}

int64_t Fight_strikes(struct Fight* this)
{
    return this->hits(this) + this->avoided(this);
}

int64_t Fight_hits(struct Fight* this)
{
    if(this->m_byVerb.size)
    {
        int64_t total = 0;
        for(size_t i = 0; i < this->m_byVerb.size; i++)
        {
            struct ByVerb* byVerb = (struct ByVerb*)this->m_byVerb.at(&this->m_byVerb, i);
            total += byVerb->m_melee.hits(&byVerb->m_melee);
        }
        return total;
    }
    return this->m_melee.hits(&this->m_melee);
}

int64_t Fight_damage(struct Fight* this)
{
    if(this->m_byVerb.size > 0)
    {
        int64_t total = 0;
        for(size_t i = 0; i < this->m_byVerb.size; i++)
        {
            struct ByVerb* byVerb = (struct ByVerb*)this->m_byVerb.at(&this->m_byVerb, i);
            total += byVerb->m_melee.damage(&byVerb->m_melee);
        }
        return total;
    }
    return this->m_melee.damage(&this->m_melee);
}

int64_t Fight_avoided(struct Fight* this)
{
    if(this->m_byVerb.size > 0)
    {
        int64_t total = 0;
        for(size_t i = 0; i < this->m_byVerb.size; i++)
        {
            struct ByVerb* byVerb = (struct ByVerb*)this->m_byVerb.at(&this->m_byVerb, i);
            total += byVerb->m_melee.avoided(&byVerb->m_melee);
        }
        return total;
    }
    return this->m_melee.avoided(&this->m_melee);
}

void Battle_String_dtor(void* string)
{
    struct String* deleteme = (struct String*) string;
    deleteme->dtor(deleteme);
}

void Battle_ctor(struct Battle* this)
{
    *this = (struct Battle) {
        .m_start = -1,
        .m_end = -1,
        .m_expire = 0,
        .m_totalHeals = 0,
        .m_lastCrit = 0,
        .m_lastCrip = 0,
        .m_lastHolyBlade = 0,
        .m_lastFinishingBlow = -1,
        .start = &Battle_start,
        .reset = &Battle_reset,
        .inProgress = &Battle_inProgress,
        .isOver = &Battle_isOver,
        .seconds = &Battle_seconds,
        .report = &Battle_report,
        .melee = &Battle_melee,
        .miss = &Battle_miss,
        .crit = &Battle_crit,
        .crip = &Battle_crip,
        .holyBlade = &Battle_holyBlade,
        .magic = &Battle_magic,
        .heal = &Battle_heal,
        .death = &Battle_death,
        .sort = &Battle_sort,
        .dtor = &Battle_dtor
    };
    Array_ctor(&this->m_pc, sizeof(struct String));
    Array_ctor(&this->m_fight, sizeof(struct Fight));
    this->m_fight.datum_dtor = Fight_dtor;
    Array_ctor(&this->m_death, sizeof(struct Death));
    this->m_pc.datum_dtor = Battle_String_dtor;
}

struct Battle* Battle_new()
{
    struct Battle* this = (struct Battle*)malloc(sizeof(struct Battle));
    Battle_ctor(this);
    return this;
}

// Member function implementations
int64_t MeleeDamage_swings(struct MeleeDamage* this)
{
    return this->strikes(this) + this->misses(this);
}

int64_t MeleeDamage_misses(struct MeleeDamage* this)
{
    return this->m_misses;
}

int64_t MeleeDamage_strikes(struct MeleeDamage* this)
{
    return this->hits(this) + this->avoided(this);
}

int64_t MeleeDamage_avoided(struct MeleeDamage* this)
{
    return this->m_parries
        + this->m_dodges
        + this->m_ripostes
        + this->m_blocks
        + this->m_absorb;
}

int64_t MeleeDamage_hits(struct MeleeDamage* this)
{
    return this->m_hits
        + this->m_crit_hits
        + this->m_crip_hits
        + this->m_holy_hits;
}

int64_t MeleeDamage_damage(struct MeleeDamage* this)
{
    return this->m_damage
        + this->m_crit_damage
        + this->m_crip_damage
        + this->m_holy_damage;
}

void Fight_dtor(void* _this)
{
    struct Fight* this = (struct Fight*)_this;
    this->m_byVerb.dtor(&this->m_byVerb);
}

void Battle_dtor(struct Battle *this)
{
    // Clear out all of the Strings
    this->m_pc.dtor(&this->m_pc);
    this->m_fight.dtor(&this->m_fight);
    this->m_death.dtor(&this->m_death);
}

void Battle_start(struct Battle* this, int64_t now)
{
    if(!this->inProgress(this))
    {
        this->m_start = now;
        Battle_keepalive(this, now);
    }
}

void Battle_keepalive(struct Battle* this, int64_t now)
{
    this->m_end = now;
    this->m_expire = now + configInstance()->keepAlive;
}

bool Battle_isOver(struct Battle* this, int64_t now)
{
    return (this->m_start > -1 && this->m_expire < now);
}

bool Battle_inProgress(struct Battle* this)
{
    return this->seconds(this) > 0;
}

int64_t Battle_seconds(struct Battle* this)
{
    if(this->m_start == -1)
        return 0;
    if(this->m_end == this->m_start)
        return 1;
    return this->m_end - this->m_start;
}

void Battle_reset(struct Battle* this)
{
    this->m_start = -1;
    this->m_end = -1;
    this->m_expire = 0;
    this->m_totalHeals = 0;
    this->m_lastCrit = 0;
    this->m_lastCrip = 0;
    this->m_lastHolyBlade = 0;
    this->m_lastFinishingBlow = -1;
    this->m_pc.clear(&this->m_pc);
    this->m_fight.clear(&this->m_fight);
    this->m_death.clear(&this->m_death);
}

void print_extra_extra(const char* type, int64_t hits, int64_t damage, int64_t fightSeconds)
{
    printf("%-35s      *%-24s %4s %4"PRId64" %5.2f %6"PRId64" %6.2f %7.2f\n",
        "",
        type,
        "",
        hits,
        ratio(hits, fightSeconds),
        damage,
        ratio(damage, hits),
        ratio(damage, fightSeconds)
    );
}

void print_extra(const struct SimpleString* type, struct MeleeDamage* melee, int64_t fightSeconds, bool printMisses)
{
    printf("%-35s   *%-27.*s %4s %4"PRId64" %5.2f %6"PRId64" %6.2f %7.2f\n",
        "",
        (int)type->length, type->data,
        "",
        melee->m_hits,
        ratio(melee->m_hits, fightSeconds),
        melee->m_damage,
        ratio(melee->m_damage, melee->m_hits),
        ratio(melee->m_damage, fightSeconds)
    );
    if(melee->m_crit_hits > 0)
        print_extra_extra("critical hits", melee->m_crit_hits, melee->m_crit_damage, fightSeconds);
    if(melee->m_crip_hits > 0)
        print_extra_extra("crippling blows", melee->m_crip_hits, melee->m_crip_damage, fightSeconds);
    if(melee->m_holy_hits > 0)
        print_extra_extra("holy blade", melee->m_holy_hits, melee->m_holy_damage, fightSeconds);

    if(!printMisses)
        return;

    int64_t swings = melee->swings(melee);
    int64_t strikes = melee->strikes(melee);
    int64_t avoided = melee->avoided(melee);
    int64_t misses = melee->misses(melee);
    if(misses || avoided)
    {
        printf("    *swings     %5"PRId64"\n", swings);
        printf("    *misses     %5"PRId64" %5.2f%% \n", misses, percent(misses, swings));
        printf("    *strikes    %5"PRId64" %5.2f%% \n", strikes, percent(strikes, swings));
        printf("    *avoided    %5"PRId64" %5.2f%% \n", avoided, percent(avoided, swings));
        if(melee->m_parries)
            printf("      *parries  %5"PRId64" %5.2f%% \n", melee->m_parries, percent(melee->m_parries, swings));
        if(melee->m_dodges)
            printf("      *dodges   %5"PRId64" %5.2f%% \n", melee->m_dodges, percent(melee->m_dodges, swings));
        if(melee->m_ripostes)
            printf("      *ripostes %5"PRId64" %5.2f%% \n", melee->m_ripostes, percent(melee->m_ripostes, swings));
        if(melee->m_blocks)
            printf("      *blocks   %5"PRId64" %5.2f%% \n", melee->m_blocks, percent(melee->m_blocks, swings));
        if(melee->m_absorb)
            printf("      *absorb   %5"PRId64" %5.2f%% \n", melee->m_absorb, percent(melee->m_absorb, swings));
    }
}

void Battle_report(struct Battle* this)
{
    char datebuf[24] = {0};
    struct SimpleString date = { .data = datebuf, .length = sizeof(datebuf) };
    struct Config* config = configInstance();
    int64_t battleSeconds = this->m_end - this->m_start;
    if(battleSeconds == 0)
        battleSeconds = 1;
    printf("===============  Battle report! %6"PRId64"s [", battleSeconds);
    unparseDate(this->m_start, &date);
    printf("%.*s", (int)date.length, date.data);
    printf(" - ");
    unparseDate(this->m_end, &date);
    printf("%.*s", (int)date.length, date.data);
    printf("]   ================\n");
    #define break_str "--------------------------------------------------------------------------------------------------------"
    #define header_format "%-35s %-30s %4s %4s %5s %6s %6s %6s\n"
    #define melee_format "%-35.*s %-30.*s %4"PRId64" %4"PRId64" %5.2f %6"PRId64" %6.2f %7.2f\n"
    #define total_format "%-35s %-30s %4"PRId64" %4"PRId64" %5.2f %6"PRId64" %6.2f %7.2f\n"
    #define death_format "%-35.*s %-30.*s %s\n"
    printf(header_format, "(N)PC", "Target", "Sec", "Hits", "h/s", "Damage", "d/h", "d/s");
    printf("%s\n", break_str);
    Battle_sort(this);
    for(int64_t pcId = 0; pcId < this->m_pc.size; pcId++)
    {
        int fightsReported = 0;
        int64_t totalHits = 0;
        int64_t totalDamage = 0;
        for(size_t i = 0; i < this->m_fight.size; i++)
        {
            struct Fight* fight = this->m_fight.at(&this->m_fight, i);
            int64_t id = config->reportByTarget ? fight->targetId : fight->sourceId;
            bool printSource = (!fightsReported ||  config->reportByTarget);
            bool printTarget = (!fightsReported || !config->reportByTarget);
            if(id != pcId)
                continue;
            // TODO: Handle Ids that are less than 0
            struct String* source = this->m_pc.at(&this->m_pc, fight->sourceId);
            struct String* target = this->m_pc.at(&this->m_pc, fight->targetId);
            int64_t fightSeconds = fight->seconds(fight);
            int64_t fightHits = fight->hits(fight);
            int64_t fightDamage = fight->damage(fight);
            printf(melee_format,
                (int)source->length, printSource ? source->data : "",
                (int)target->length, printTarget ? target->data : "",
                fightSeconds,
                fightHits,
                ratio(fightHits, fightSeconds),
                fightDamage,
                ratio(fightDamage, fightHits),
                ratio(fightDamage, fightSeconds)
            );
            totalHits += fightHits;
            totalDamage += fightDamage;
            fightsReported++;
            if(config->extra)
            {
                printf("%-35s %-30s %4s %4"PRId64" %5.2f\n",
                    "",
                    "  *swings",
                    "",
                    fight->swings(fight),
                    ratio(fight->swings(fight), fightSeconds)
                );
                if(config->byVerb)
                {
                    for(size_t i = 0; i < fight->m_byVerb.size; i++)
                    {
                        struct ByVerb* byVerb = (struct ByVerb*)fight->m_byVerb.at(&fight->m_byVerb, i);
                        print_extra(byVerb->m_verb.to_SimpleString(&byVerb->m_verb), &byVerb->m_melee, fightSeconds, config->misses);
                    }
                }
                else
                {
                    struct SimpleString _melee = SIMPLE_STRING("melee");
                    print_extra(&_melee, &fight->m_melee, fightSeconds, config->misses);
                }
                if(fight->m_magic.hits > 0)
                {
                    printf("%-35s %-30s %4s %4"PRId64" %5.2f %6"PRId64" %6.2f %7.2f\n",
                        "",
                        "  *spell/ds",
                        "",
                        fight->m_magic.hits,
                        ratio(fight->m_magic.hits, fightSeconds),
                        fight->m_magic.damage,
                        ratio(fight->m_magic.damage, fight->m_magic.hits),
                        ratio(fight->m_magic.damage, fightSeconds)
                    );
                }
            }
            else
            {
                if(config->byVerb)
                {
                    for(size_t i = 0; i < fight->m_byVerb.size; i++)
                    {
                        struct ByVerb* byVerb = (struct ByVerb*)fight->m_byVerb.at(&fight->m_byVerb, i);
                        print_extra(byVerb->m_verb.to_SimpleString(&byVerb->m_verb), &byVerb->m_melee, fightSeconds, config->misses);
                    }
                }
            }
        }
        if(fightsReported > 0)
        {
            printf(total_format,
                "",
                "  *battle",
                battleSeconds,
                totalHits,
                (double)totalHits / (double)battleSeconds,
                totalDamage,
                totalHits ? (double)totalDamage / (double)totalHits : 0,
                (double)totalDamage / (double)battleSeconds
            );
            printf("%s\n", break_str);
        }
    }
    if(this->m_totalHeals > 0)
    {
        printf("Total Heals: %"PRId64"\n", this->m_totalHeals);
        printf("%s\n", break_str);
    }
    if(this->m_death.size > 0)
    {
        printf("Deaths: %zu\n", this->m_death.size);
        printf(death_format, 6, "Target", 6, "Slayer", "");
        printf("%s\n", break_str);
        for(size_t i = 0; i < this->m_death.size; i++)
        {
            struct Death* death = this->m_death.at(&this->m_death, i);
            struct String* source = this->m_pc.at(&this->m_pc, death->sourceId);
            struct String* target = this->m_pc.at(&this->m_pc, death->targetId);
            printf(death_format,
                (int)target->length, target->data,
                source ? (int)source->length : 0, source ? source->data : "",
                death->finishingBlow ? "Finishing Blow" : "")
            ;
        }
        printf("%s\n", break_str);
    }
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
        String_ctorCopyString(&saveme, findme);
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
    this->start(this, now);
    struct Config* config = configInstance();
    int64_t sourceId = Battle_getPCIndex(this, &action->source);
    int64_t targetId = Battle_getPCIndex(this, &action->target);
    // TODO: Handle finishing blows separately
    // Preceeded by a "<Name> scores a Finishing Blow!!"
    if(action->type == MELEE && action->damage >= 32000)
    {
        this->m_lastFinishingBlow = targetId;
        return;
    }
    struct Fight* fight = Battle_getFightIndex(this, now, sourceId, targetId);
    fight->end = now;

    struct MeleeDamage *melee = fight->getMeleeDamage(fight, action->verb.to_SimpleString(&action->verb));

    if(config->extra)
    {
        if(this->m_lastCrit == action->damage)
        {
            melee->m_crit_hits++;
            melee->m_crit_damage += action->damage;
            this->m_lastCrit = 0;
        }
        else if(this->m_lastCrip == action->damage)
        {
            melee->m_crip_hits++;
            melee->m_crip_damage += action->damage;
            this->m_lastCrip = 0;
        }
        else if(this->m_lastHolyBlade == action->damage)
        {
            melee->m_holy_hits++;
            melee->m_holy_damage += action->damage;
            this->m_lastHolyBlade = 0;
        }
        else
        {
            melee->m_hits++;
            melee->m_damage += action->damage;
        }
    }
    else
    {
        melee->m_hits++;
        melee->m_damage += action->damage;
    }

    Battle_keepalive(this, now);
}

void Battle_miss(struct Battle* this, int64_t now, struct Action* action)
{
    this->start(this, now);
    int64_t sourceId = Battle_getPCIndex(this, &action->source);
    int64_t targetId = Battle_getPCIndex(this, &action->target);
    struct Fight* fight = Battle_getFightIndex(this, now, sourceId, targetId);
    struct MeleeDamage *melee = fight->getMeleeDamage(fight, action->verb.to_SimpleString(&action->verb));

    switch(action->type)
    {
        case PARRY:
            melee->m_parries++;
            break;
        case DODGE:
            melee->m_dodges++;
            break;
        case RIPOSTE:
            melee->m_ripostes++;
            break;
        case BLOCK:
            melee->m_blocks++;
            break;
        case ABSORB:
            melee->m_absorb++;
            break;
        case MISS:
        default:
            melee->m_misses++;
            break;
    }
    fight->end = now;
    Battle_keepalive(this, now);
}

void Battle_crit(struct Battle* this, int64_t now, struct Action* action)
{
    this->m_lastCrit = action->damage;
}

void Battle_crip(struct Battle* this, int64_t now, struct Action* action)
{
    this->m_lastCrip = action->damage;
}

void Battle_holyBlade(struct Battle* this, int64_t now, struct Action* action)
{
    this->m_lastHolyBlade = action->damage;
}

void Battle_magic(struct Battle* this, int64_t now, struct Action* action)
{
    Battle_melee(this, now, action);

    this->start(this, now);
    int64_t sourceId = Battle_getPCIndex(this, &action->source);
    int64_t targetId = Battle_getPCIndex(this, &action->target);

    struct Fight* fight = Battle_getFightIndex(this, now, sourceId, targetId);
    fight->m_magic.hits++;
    fight->m_magic.damage += action->damage;
    fight->end = now;

    Battle_keepalive(this, now);
}

void Battle_heal(struct Battle* this, int64_t now, struct Action* action)
{
    this->start(this, now);
    this->m_totalHeals += action->damage;
    Battle_keepalive(this, now);
}

void Battle_death(struct Battle* this, int64_t now, struct Action* action)
{
    this->start(this, now);
    struct Death death = {0};
    death.sourceId = Battle_getPCIndex(this, &action->source);
    death.targetId = Battle_getPCIndex(this, &action->target);
    if(death.targetId == this->m_lastFinishingBlow)
    {
        death.finishingBlow = true;
        this->m_lastFinishingBlow = -1;
    }
    this->m_death.push(&this->m_death, &death);
    Battle_keepalive(this, now);
}

void Battle_sort(struct Battle* this)
{
    // Sort fight by dps
    // Then go through and sort pcIds by where they appear in the fight dps.
    if (this->m_fight.size < 2)
        return;
    for(size_t i = 0; i < this->m_fight.size - 1; i++)
    {
        for(size_t j = 0; j < this->m_fight.size - i - 1; j++)
        {
            struct Fight* left = this->m_fight.at(&this->m_fight, j);
            struct Fight* right = this->m_fight.at(&this->m_fight, j+1);
            if (ratio(left->m_melee.damage(&left->m_melee), left->seconds(left)) < ratio(right->m_melee.damage(&right->m_melee), right->seconds(right)))
            {
                this->m_fight.swap(&this->m_fight, j, j+1);
            }
        }
    }
}
