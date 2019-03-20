#include "action.h"
#include "array.h"
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
};
void parseVerb(struct Action* this, struct String message)
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
                this->target.hold(&this->target, message.data, 3); // These lines begin with "You"
                this->verb.hold(&this->verb, aeVerb.data + 1, aeVerb.length - 2); // Trim leading/trailing slash
                this->damage = damage.toInt(&damage);
                damage.dtor(&damage);
                return;
            }
            break;
        }
    }
    // TODO: Measure this, does it take a time to build this on the stack?
    static struct SimpleString verbs[] = {
        SIMPLE_STRING(" backstab "),
        SIMPLE_STRING(" backstabs "),
        SIMPLE_STRING(" bash "),
        SIMPLE_STRING(" bashes "),
        SIMPLE_STRING(" bites "),
        SIMPLE_STRING(" claw "),
        SIMPLE_STRING(" claws "),
        SIMPLE_STRING(" crush "),
        SIMPLE_STRING(" crushes "),
        SIMPLE_STRING(" gore "),
        SIMPLE_STRING(" gores "),
        SIMPLE_STRING(" hit "),
        SIMPLE_STRING(" hits "),
        SIMPLE_STRING(" kick "),
        SIMPLE_STRING(" kicks "),
        SIMPLE_STRING(" maul "),
        SIMPLE_STRING(" mauls "),
        SIMPLE_STRING(" pierce "),
        SIMPLE_STRING(" pierces "),
        SIMPLE_STRING(" punch "),
        SIMPLE_STRING(" punches "),
        SIMPLE_STRING(" rend "),
        SIMPLE_STRING(" rends "),
        SIMPLE_STRING(" slash "),
        SIMPLE_STRING(" slashes "),
        SIMPLE_STRING(" slice "),
        SIMPLE_STRING(" slices "),
        SIMPLE_STRING(" smash "),
        SIMPLE_STRING(" smashes "),
    };
    struct SimpleString nonMeleeVerb = SIMPLE_STRING(" was hit by ");
    struct SimpleString healedVerb = SIMPLE_STRING(" have been healed ");
    // 0 initial
    // 1 found melee verb, looking for target length
    // 2 found non-melee verb, looking for damage value
    //   found target length, looking for damage value
    int state = Split0;
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
            if(state == Split0)
            {
                if(here.startsWith(&here, &nonMeleeVerb))
                {
                    this->type = MAGIC;
                    this->target.data = message.data;
                    this->target.length = i;
                    this->verb.data = message.data + i + nonMeleeVerb.length;
                    this->verb.length = 9;
                    i += nonMeleeVerb.length;
                    state = Split2;
                }
                else if(here.startsWith(&here, &healedVerb))
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
                        if(here.startsWith(&here, &verbs[j]))
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
                struct SimpleString _for_ = SIMPLE_STRING(" for ");
                if(here.startsWith(&here, &_for_))
                {
                    if(state == Split1)
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
 */

void Action_parse(struct Action* this, struct String message)
{
    static struct SimpleString pointsOfDamage = SIMPLE_STRING(" points of damage.");
    static struct SimpleString pointOfDamage = SIMPLE_STRING(" point of damage.");
    if(message.endsWith(&message, &pointsOfDamage))
    {
        message.length -= pointsOfDamage.length;
        parseVerb(this, message);
    }
    else if(message.endsWith(&message, &pointOfDamage))
    {
        message.length -= pointOfDamage.length;
        parseVerb(this, message);
    }
}
