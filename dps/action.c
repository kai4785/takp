#include "action.h"
#include "utility.h"

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
    Split0,
    Split1,
    Split2,
    Split3,
    Split4,
    Split5,
};
void parseVerb(struct Action* this, struct String message)
{
    // TODO: Measure this, does it take a time to build this on the stack?
    struct String verbs[] = {
        CONST_STRING(" backstab "),
        CONST_STRING(" backstabs "),
        CONST_STRING(" bash "),
        CONST_STRING(" bashes "),
        CONST_STRING(" bites "),
        CONST_STRING(" claw "),
        CONST_STRING(" claws "),
        CONST_STRING(" crush "),
        CONST_STRING(" crushes "),
        CONST_STRING(" gore "),
        CONST_STRING(" gores "),
        CONST_STRING(" hit "),
        CONST_STRING(" hits "),
        CONST_STRING(" kick "),
        CONST_STRING(" kicks "),
        CONST_STRING(" maul "),
        CONST_STRING(" mauls "),
        CONST_STRING(" pierce "),
        CONST_STRING(" pierces "),
        CONST_STRING(" punch "),
        CONST_STRING(" punches "),
        CONST_STRING(" rend "),
        CONST_STRING(" rends "),
        CONST_STRING(" slash "),
        CONST_STRING(" slashes "),
        CONST_STRING(" slice "),
        CONST_STRING(" slices "),
        CONST_STRING(" smash "),
        CONST_STRING(" smashes "),
    };
    const struct String nonMeleeVerb = CONST_STRING(" was hit by ");
    const struct String healedVerb = CONST_STRING(" have been healed ");
    // 0 initial
    // 1 found melee verb, looking for target length
    // 2 found non-melee verb, looking for damage value
    //   found target length, looking for damage value
    int state = Split0;
    struct String damage = {0};
    for(size_t i = 0; i < message.length; i++)
    {
        // If we found the start of a word
        if(message.data[i] == ' ')
        {
            struct String here = {
                .data = message.data + i,
                .length = message.length - i,
            };
            if(state == Split0)
            {
                if(startsWith(here, nonMeleeVerb))
                {
                    this->type = MAGIC;
                    this->target.data = message.data;
                    this->target.length = i;
                    this->verb.data = message.data + i + nonMeleeVerb.length;
                    this->verb.length = 9;
                    i += nonMeleeVerb.length;
                    state = Split2;
                }
                else if(startsWith(here, healedVerb))
                {
                    this->type = HEAL;
                    this->target.data = message.data;
                    this->target.length = i;
                    this->verb.data = message.data + i + 1;
                    this->verb.length = healedVerb.length - 2; // No spaces
                    i += healedVerb.length - 2;
                    state = Split2;
                }
                else
                {
                    for(size_t j = 0; j < ARRAY_SIZE(verbs); j++)
                    {
                        if(startsWith(here, verbs[j]))
                        {
                            // We found it!
                            this->type = MELEE;
                            this->source.data = message.data;
                            this->source.length = i;
                            this->verb.data = message.data + i + 1; // Skip the space
                            this->verb.length = verbs[j].length - 2; // No spaces
                            this->target.data = this->verb.data + this->verb.length + 1; // Skip the space
                            // Still need target length;
                            state = Split1;
                            i += verbs[j].length;
                            break;
                        }
                    }
                }
            }
            else if(state == Split1 || state == Split2)
            {
                // Now we're looking for the end of target, by searching for "for"
                struct String _for_ = CONST_STRING(" for ");
                if(startsWith(here, _for_))
                {
                    if(state == Split1)
                        this->target.length = message.data + i - this->target.data;
                    damage.data = message.data + i + _for_.length;
                    state = Split3;
                }
            }
            else if(state == Split3)
            {
                struct String _point = CONST_STRING(" point");
                if(startsWith(here, _point))
                {
                    damage.length = message.data + i - damage.data;
                    this->damage = toInt(damage);
                    break;
                }
            }
        }
    }
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
 */

void Action_parse(struct Action* this, struct String message)
{
    const struct String pointsOfDamage = CONST_STRING("points of damage.");
    const struct String pointOfDamage = CONST_STRING("point of damage.");
    if(endsWith(message, pointsOfDamage) || endsWith(message, pointOfDamage))
    {
        parseVerb(this, message);
    }
}
