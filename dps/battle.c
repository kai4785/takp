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

// Member function declarations
// Fight
void Fight_dtor(struct Fight *this);
int64_t Fight_seconds(struct Fight *this);
// Battle
void Battle_dtor(struct Battle *this);
void Battle_reset(struct Battle *this);
void Battle_start(struct Battle* this, int64_t now);
void Battle_keepalive(struct Battle* this, int64_t now);
bool Battle_inProgress(struct Battle *this);
bool Battle_isOver(struct Battle *this, int64_t now);
int64_t Battle_seconds(struct Battle *this);
void Battle_report(struct Battle* this);
void Battle_melee(struct Battle* this, int64_t now, struct Action* action);
void Battle_crit(struct Battle* this, int64_t now, struct Action* action);
void Battle_crip(struct Battle* this, int64_t now, struct Action* action);
void Battle_holyBlade(struct Battle* this, int64_t now, struct Action* action);
void Battle_magic(struct Battle* this, int64_t now, struct Action* action);
void Battle_heal(struct Battle* this, int64_t now, struct Action* action);
void Battle_death(struct Battle* this, int64_t now, struct Action* action);
void Battle_sort(struct Battle* this);

// Helper Functions
double dps(struct Damage damage, int64_t seconds)
{
    return (double)(damage.damage) / (double)(seconds);
}

double hps(struct Damage damage, int64_t seconds)
{
    return (double)(damage.hits) / (double)(seconds);
}

double dph(struct Damage damage)
{
    if(damage.hits)
        return (double)(damage.damage) / (double)(damage.hits);
    else
        return 0;
}

// Constructors
void Fight_ctor(struct Fight* this)
{
    *this = (struct Fight){
        .sourceId = 0,
        .targetId = 0,
        .start = 0,
        .end = 0,
        .m_melee = {0},
        .m_backstab = {0},
        .m_crit = {0},
        .m_crip = {0},
        .m_magic = {0},
        .seconds = &Fight_seconds,
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
void Fight_dtor(struct Fight *this)
{
    (void)this;
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
    #define backstab_format "%-35s %-30s %4s %4"PRId64" %5.2f %6"PRId64" %6.2f %7.2f\n"
    #define crit_format backstab_format
    #define crip_format backstab_format
    #define holyBlade_format backstab_format
    #define magic_format backstab_format
    #define total_format "%-35s %-30s %4"PRId64" %4"PRId64" %5.2f %6"PRId64" %6.2f %7.2f\n"
    #define death_format "%-35.*s %-30.*s %s\n"
    printf(header_format, "(N)PC", "Target", "Sec", "Hits", "h/s", "Damage", "d/h", "d/s");
    printf("%s\n", break_str);
    Battle_sort(this);
    for(int64_t pcId = 0; pcId < this->m_pc.size; pcId++)
    {
        int linesPrinted = 0;
        int64_t totalHits = 0;
        int64_t totalDamage = 0;
        for(size_t i = 0; i < this->m_fight.size; i++)
        {
            struct Fight* fight = this->m_fight.at(&this->m_fight, i);
            int64_t id = config->reportByTarget ? fight->targetId : fight->sourceId;
            bool printSource = (!linesPrinted ||  config->reportByTarget);
            bool printTarget = (!linesPrinted || !config->reportByTarget);
            if(id != pcId)
                continue;
            // TODO: Handle Ids that are less than 0
            struct String* source = this->m_pc.at(&this->m_pc, fight->sourceId);
            struct String* target = this->m_pc.at(&this->m_pc, fight->targetId);
            int64_t fightSeconds = fight->seconds(fight);
            printf(melee_format,
                (int)source->length, printSource ? source->data : "",
                (int)target->length, printTarget ? target->data : "",
                fightSeconds,
                fight->m_melee.hits,
                hps(fight->m_melee, fightSeconds),
                fight->m_melee.damage,
                dph(fight->m_melee),
                dps(fight->m_melee, fightSeconds)
            );
            totalHits += fight->m_melee.hits;
            totalDamage += fight->m_melee.damage;
            linesPrinted++;
            if(config->extra)
            {
                if(fight->m_backstab.hits > 0)
                {
                    printf(backstab_format,
                        "",
                        "  *backstabs",
                        "",
                        fight->m_backstab.hits,
                        hps(fight->m_backstab, fightSeconds),
                        fight->m_backstab.damage,
                        dph(fight->m_backstab),
                        dps(fight->m_backstab, fightSeconds)
                    );
                    linesPrinted++;
                }
                if(fight->m_crit.hits > 0)
                {
                    printf(crit_format,
                        "",
                        "  *critical hits",
                        "",
                        fight->m_crit.hits,
                        hps(fight->m_crit, fightSeconds),
                        fight->m_crit.damage,
                        dph(fight->m_crit),
                        dps(fight->m_crit, fightSeconds)
                    );
                    linesPrinted++;
                }
                if(fight->m_crip.hits > 0)
                {
                    printf(crip_format,
                        "",
                        "  *crippling blows",
                        "",
                        fight->m_crip.hits,
                        hps(fight->m_crip, fightSeconds),
                        fight->m_crip.damage,
                        dph(fight->m_crip),
                        dps(fight->m_crip, fightSeconds)
                    );
                    linesPrinted++;
                }
                if(fight->m_holyBlade.hits > 0)
                {
                    printf(holyBlade_format,
                        "",
                        "  *holy blade",
                        "",
                        fight->m_holyBlade.hits,
                        hps(fight->m_holyBlade, fightSeconds),
                        fight->m_holyBlade.damage,
                        dph(fight->m_holyBlade),
                        dps(fight->m_holyBlade, fightSeconds)
                    );
                    linesPrinted++;
                }
                if(fight->m_magic.hits > 0)
                {
                    printf(magic_format,
                        "",
                        "  *spell/ds",
                        "",
                        fight->m_magic.hits,
                        hps(fight->m_magic, fightSeconds),
                        fight->m_magic.damage,
                        dph(fight->m_magic),
                        dps(fight->m_magic, fightSeconds)
                    );
                    linesPrinted++;
                }
            }
        }
        if(linesPrinted > 0)
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
    fight->m_melee.hits++;
    fight->m_melee.damage += action->damage;
    fight->end = now;
    if(action->verb.cmp(&action->verb, "backstab", 8))
    {
        fight->m_backstab.hits++;
        fight->m_backstab.damage += action->damage;
    }
    else if(this->m_lastCrit == action->damage)
    {
        fight->m_crit.hits++;
        fight->m_crit.damage += action->damage;
        this->m_lastCrit = 0;
    }
    else if(this->m_lastCrip == action->damage)
    {
        fight->m_crip.hits++;
        fight->m_crip.damage += action->damage;
        this->m_lastCrip = 0;
    }
    else if(this->m_lastHolyBlade == action->damage)
    {
        fight->m_holyBlade.hits++;
        fight->m_holyBlade.damage += action->damage;
        this->m_lastHolyBlade = 0;
    }

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
            if (dps(left->m_melee, left->seconds(left)) < dps(right->m_melee, right->seconds(right)))
            {
                this->m_fight.swap(&this->m_fight, j, j+1);
            }
        }
    }
}
