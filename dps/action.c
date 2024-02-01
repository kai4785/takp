#include "action.h"
#include "array.h"
#include "utility.h"
#include "config.h"

#include <string.h>

// Member function declarations
void Action_dtor(struct Action* this);
void Action_parse(struct Action* this, struct String message);

// Constructors
void Action_ctor(struct Action* this)
{
    *this = (struct Action){
        .type = UNKNOWN,
        .damage = 0,
        .message = NULL,
        .parse = &Action_parse,
        .dtor = &Action_dtor,
    };
    String_ctor(&(this->source));
    String_ctor(&(this->target));
    String_ctor(&(this->verb));
}

// Member function implementations
void Action_dtor(struct Action* this)
{
    this->source.dtor(&(this->source));
    this->target.dtor(&(this->target));
    this->verb.dtor(&(this->verb));
}

enum {
    State0,
    State1,
    State2,
    State3,
};

struct Verb ActionVerbs[15] = {
    {
        {" backstab " , 10},
        {" backstabs ", 11},
        Backstab
    },
    {
        {" bash "     ,  6},
        {" bashes "   ,  8},
        Bash
    },
    {
        {" bite "    ,  7},
        {" bites "    ,  7},
        Bite
    },
    {
        {" claw "     ,  6},
        {" claws "    ,  7},
        Claw
    },
    {
        {" crush "    ,  7},
        {" crushes "  ,  9},
        Crush
    },
    {
        {" gore "     ,  6},
        {" gores "    ,  7},
        Gore
    },
    {
        {" hit "      ,  5},
        {" hits "     ,  6},
        Hit
    },
    {
        {" kick "     ,  6},
        {" kicks "    ,  7},
        Kick
    },
    {
        {" maul "     ,  6},
        {" mauls "    ,  7},
        Maul
    },
    {
        {" pierce "   ,  8},
        {" pierces "  ,  9},
        Pierce
    },
    {
        {" punch "    ,  7},
        {" punches "  ,  9},
        Punch
    },
    {
        {" rend "     ,  6},
        {" rends "    ,  7},
        Rend
    },
    {
        {" slash "    ,  7},
        {" slashes "  ,  9},
        Slash
    },
    {
        {" slice "    ,  7},
        {" slices "   ,  8},
        Slice
    },
    {
        {" smash "    ,  7},
        {" smashes "  ,  9},
        Smash
    }
};

void parseDamage(struct Action* this, struct String message)
{
    struct String here;
    String_ctor(&here);
    struct SimpleString aeVerb = SIMPLE_STRING(" have taken ");
    // First things first, find the AE damage, cause sometimes it says "hit". It's really easy to find.
    for(size_t i = message.length - 1; i > aeVerb.length + 1; i--)
    {
        // Look for the space before the aeVerb that comes before the damage value
        if(message.data[i - aeVerb.length] == ' ')
        {
            // Make sure that the aeVerb is found immediately before the damage value
            size_t verbStart = i - aeVerb.length;
            here.hold(&here,
                message.data + verbStart,
                aeVerb.length);
            // Look for the aeVerb, if it's not found, there's no aeVerb anywhere
            if(here.op_equal(&here, &aeVerb))
            {
                struct String damage;
                String_ctorHold(&damage, message.data + i, message.length - i);
                // This is a special AE type message; we just parse it and return, or break and continue;
                this->type = MAGIC;
                this->source.hold(&this->source, "Non-melee", 9);
                struct Config* config = configInstance();
                this->target.hold(&this->target, config->me.data, config->me.length);
                this->verb.hold(&this->verb, aeVerb.data + 1, aeVerb.length - 2); // Trim leading/trailing slash
                this->damage = damage.toInt(&damage);
                damage.dtor(&damage);
                return;
            }
            break;
        }
    }
    struct SimpleString nonMeleeVerb1 = SIMPLE_STRING(" was hit by ");
    struct SimpleString nonMeleeVerb2 = SIMPLE_STRING(" were hit by ");
    struct SimpleString healedVerb = SIMPLE_STRING(" have been healed ");
    // 0 initial
    // 1 found melee verb, looking for target length
    // 2 found non-melee verb, looking for damage value
    //   found target length, looking for damage value
    int state = State0;
    for(size_t i = 0; i < message.length; i++)
    {
        // If we found the start of a word
        if(message.data[i] == ' ')
        {
            here.hold(
                &here,
                message.data + i,
                message.length - i
            );
            if(state == State0)
            {
                if(here.startsWith(&here, &nonMeleeVerb1) || here.startsWith(&here, &nonMeleeVerb2))
                {
                    size_t length = (here.data[2] == 'a') ? nonMeleeVerb1.length : nonMeleeVerb2.length;
                    this->type = MAGIC;
                    //this->source = CONST_STRING("Spell/DS");
                    this->source = CONST_STRING("You");
                    this->target.data = message.data;
                    this->target.length = i;
                    this->verb.data = message.data + i + length;
                    this->verb.length = 9;
                    i += length;
                    state = State2;
                }
                else if(here.startsWith(&here, &healedVerb))
                {
                    this->type = HEAL;
                    this->source.hold(&this->source, g_empty.data, g_empty.length);
                    this->target.data = message.data;
                    this->target.length = i;
                    this->verb.data = message.data + i + 1;
                    this->verb.length = healedVerb.length - 2; // No spaces
                    i += healedVerb.length - 2;
                    state = State2;
                }
                else
                {
                    for(size_t j = 0; j < ARRAY_SIZE(ActionVerbs); j++)
                    {
                        struct SimpleString found = {0};
                        if(here.startsWith(&here, &ActionVerbs[j].singular))
                            found = ActionVerbs[j].singular;
                        else if(here.startsWith(&here, &ActionVerbs[j].plural))
                            found = ActionVerbs[j].plural;

                        if(found.length)
                        {
                            // We found it!
                            this->type = MELEE;
                            this->source.data = message.data;
                            this->source.length = i;
                            this->verb.data = ActionVerbs[j].singular.data + 1; // Skip the space
                            this->verb.length = ActionVerbs[j].singular.length - 2; // No spaces
                            this->target.data = message.data + i + 1 + found.length - 1; // Skip the spaces
                            // Still need target length;
                            state = State1;
                            i += found.length;
                            break;
                        }
                    }
                }
            }
            else if(state == State1 || state == State2)
            {
                // Now we're looking for the end of target, by searching for "for"
                struct SimpleString _for_ = SIMPLE_STRING(" for ");
                if(here.startsWith(&here, &_for_))
                {
                    if(state == State1)
                        this->target.length = message.data + i - this->target.data;
                    struct String damage;
                    String_ctorHold(&damage, message.data + i + _for_.length, message.length - i - _for_.length);
                    this->damage = damage.toInt(&damage);
                    damage.dtor(&damage);
                }
            }
        }
    }
    here.dtor(&here);
}

/* Battle messages are one of the following.
 * End of Battle
      LOADING, PLEASE WAIT...
 * Melee Damage
 *    Verbs "smash|smashes|hit|slash|claw|claws|crush|pierce|kick|bash|maul|gore|gores|slice|slices|slashes|crushes|hits|punch|punches|kicks|bashes|bites|pierces|mauls|backstab|backstabs|rends"
 *    ^([A-Za-z `]+) (verb) ([A-Za-z `]+) for ([0-9]+) points? of damage\.
 * Magic Damage
 *    ^([A-Za-z `]+) was (hit by non-melee) for ([0-9]+) points? of damage\.
 *    ^You were (hit by non-melee) for ([0-9]+) damage\.
 * Cripple Damage (duplicate of Melee)
 *    ^([A-Za-z `]+) lands a Crippling Blow\!\(([0-9]+)\)
 * Critical Damage (duplicate of Melee/Magic)
 *    ^([A-Za-z `]+) Scores a critical hit\!\(([0-9]+)\)
 * Heal Damage
 *    ^(You) have been (healed) for ([0-9]+) points of damage\.
 * Death
 *    ^([A-Za-z `]+) have slain ([A-Za-z `]+)!
 *    ^([A-Za-z `]+) (has|have) been slain by ([A-Za-z `]+)!
 *    ^([A-Za-z `]+) died\.
 * Stun
 *    You are stunned!
 *    You are unstunned.
 */

void Action_parse(struct Action* this, struct String message)
{
    if(message.data[message.length - 1] == '.')
    {
        struct SimpleString pointsOfDamage = SIMPLE_STRING(" points of damage.");
        struct SimpleString pointOfDamage =  SIMPLE_STRING(" point of damage.");
        struct SimpleString died = SIMPLE_STRING(" died.");
        struct SimpleString unstunned = SIMPLE_STRING("unstunned.");
        if(message.endsWith(&message, &pointsOfDamage))
        {
            message.length -= pointsOfDamage.length;
            parseDamage(this, message);
        }
        else if(message.endsWith(&message, &pointOfDamage))
        {
            message.length -= pointOfDamage.length;
            parseDamage(this, message);
        }
        else if(message.endsWith(&message, &died))
        {
            this->type = DEATH;
            this->target.hold(&this->target,
                message.data, message.length - died.length);
            this->verb.hold(&this->verb,
                message.data + this->target.length + 1, died.length - 2);
        }
        else if(message.endsWith(&message, &unstunned))
        {
            // code
        }
    }
    else if(message.data[message.length - 1] == ')')
    {
        struct SimpleString critical_hit   = SIMPLE_STRING(" Scores a critical hit!(");
        struct SimpleString crippling_blow = SIMPLE_STRING(" lands a Crippling Blow!(");
        struct SimpleString holy_blade = SIMPLE_STRING("'s holy blade cleanses his target!(");
        struct SimpleString found = {0};
        if(message.find(&message, &critical_hit, &found) != SIZE_MAX)
        {
            this->type = CRIT;
        }
        else if(message.find(&message, &crippling_blow, &found) != SIZE_MAX)
        {
            this->type = CRIP;
        }
        else if(message.find(&message, &holy_blade, &found) != SIZE_MAX)
        {
            this->type = HOLYBLADE;
        }
        if(this->type != UNKNOWN)
        {
            this->source.hold(&this->source, message.data, (size_t)(found.data - message.data));
            size_t damageStart = this->source.length + found.length;
            size_t damageSize = message.length - 1 - damageStart;
            struct String damage = {0};
            String_ctorHold(&damage, message.data + damageStart, damageSize);
            this->damage = damage.toInt(&damage);
        }
    }
    else if(message.data[message.length - 1] == '!')
    {
        // You try to <verb> <target>, but miss!
        // You try to <verb> <target>, but <target> parries!
        // You try to <verb> <target>, but <target> dodges!
        // You try to <verb> <target>, but <target> ripostes!
        // You try to <verb> <target>, but <target> blocks!
        // You try to <verb> <target>, but <target>'s magical skin absorbs the blow!
        // <Target> tries to <verb> YOU, but misses!
        // <Target> tries to <verb> YOU, but YOU parry!
        // <Target> tries to <verb> YOU, but YOU dodge!
        // <Target> tries to <verb> YOU, but YOU riposte!
        // <Target> tries to <verb> YOU, but YOU block!
        // <Target> tries to <verb> YOU, but YOUR magical skin absorbs the blow!
        struct SimpleString try_to = SIMPLE_STRING(" try to ");
        struct SimpleString tries_to = SIMPLE_STRING(" tries to ");

        // You have slain <target>!
        // <target> has been slain by <source>
        // You have been slain by <target>
        struct SimpleString have_slain = SIMPLE_STRING(" have slain ");
        struct SimpleString has_been_slain_by = SIMPLE_STRING(" has been slain by ");
        struct SimpleString have_been_slain_by = SIMPLE_STRING(" have been slain by ");

        // You are stunned!
        struct SimpleString stunned = SIMPLE_STRING("stunned!");

        struct SimpleString found = {0};
        size_t pos = 0;
        if((pos = message.find(&message, &try_to, &found)) != SIZE_MAX ||
           (pos = message.find(&message, &tries_to, &found)) != SIZE_MAX)
        {
            struct SimpleString space = SIMPLE_STRING(" ");
            struct SimpleString _but = SIMPLE_STRING(", but");

            struct SimpleString miss = SIMPLE_STRING(" miss!");
            struct SimpleString parries = SIMPLE_STRING(" parries!");
            struct SimpleString dodges = SIMPLE_STRING(" dodges!");
            struct SimpleString ripostes = SIMPLE_STRING(" ripostes!");
            struct SimpleString blocks = SIMPLE_STRING(" blocks!");

            struct SimpleString misses = SIMPLE_STRING(" misses!");
            struct SimpleString parry = SIMPLE_STRING(" parry!");
            struct SimpleString dodge = SIMPLE_STRING(" dodge!");
            struct SimpleString riposte = SIMPLE_STRING(" riposte!");
            struct SimpleString block = SIMPLE_STRING(" block!");
            struct SimpleString absorb = SIMPLE_STRING(" magical skin absorbs the blow!");

            this->source.hold(&this->source, message.data, pos);

            this->verb.hold(&this->verb,
                found.data + found.length,
                message.length - pos - found.length);
            pos = this->verb.find(&this->verb, &space, &found);
            this->verb.length = pos;

            // Target starts after verb and the following space
            // Target ends where _but begins
            pos = message.find(&message, &_but, &found);
            this->target.hold(&this->target,
                this->verb.data + this->verb.length + 1,
                (size_t)(found.data - this->verb.data) - this->verb.length - 1);

            if(message.endsWith(&message, &miss) ||
               message.endsWith(&message, &misses))
            {
                this->type = MISS;
            }
            else if(message.endsWith(&message, &parries) ||
                    message.endsWith(&message, &parry))
            {
                this->type = PARRY;
            }
            else if(message.endsWith(&message, &dodges) ||
                    message.endsWith(&message, &dodge))
            {
                this->type = DODGE;
            }
            else if(message.endsWith(&message, &ripostes) ||
                    message.endsWith(&message, &riposte))
            {
                this->type = RIPOSTE;
            }
            else if(message.endsWith(&message, &blocks) ||
                    message.endsWith(&message, &block))
            {
                this->type = BLOCK;
            }
            else if(message.endsWith(&message, &absorb))
            {
                this->type = ABSORB;
            }

        }
        else if(message.find(&message, &have_slain, &found) != SIZE_MAX)
        {
            this->type = DEATH;
            this->source.hold(&this->source,
                message.data, (size_t)(found.data - message.data));
            this->target.hold(&this->target,
                message.data + this->source.length + found.length,
                message.length - 1 - this->source.length - found.length);
            this->verb.hold(&this->verb, found.data + 1, found.length - 2);
        }
        else if(
            (message.find(&message, &has_been_slain_by, &found) != SIZE_MAX) ||
            (message.find(&message, &have_been_slain_by, &found) != SIZE_MAX))
        {
            this->type = DEATH;
            this->target.hold(&this->target,
                message.data, (size_t)(found.data - message.data));
            this->source.hold(&this->source,
                message.data + this->target.length + found.length,
                message.length - 1 - this->target.length - found.length);
            this->verb.hold(&this->verb, found.data + 1, found.length - 2);
        }
        else if(message.endsWith(&message, &stunned))
        {
        }
    }
}
