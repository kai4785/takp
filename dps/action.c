#include "action.h"
#include "utility.h"

#include <string.h>

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

enum {
    Split0,
    Split1,
    Split2,
    Split3,
    Split4,
    Split5,
};
void splitVerb(struct String message, struct Action* action)
{
    // TODO: Handle the "error"
    if(!action)
        return;
    static const struct String verbs[] = {
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
    bool foundverb = false;
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
                    action->type = MAGIC;
                    action->target.data = message.data;
                    action->target.length = i;
                    action->verb.data = message.data + i + nonMeleeVerb.length;
                    action->verb.length = 9;
                    i += nonMeleeVerb.length;
                    state = Split2;
                }
                else if(startsWith(here, healedVerb))
                {
                    action->type = HEAL;
                    action->target.data = message.data;
                    action->target.length = i;
                    action->verb.data = message.data + i + 1;
                    action->verb.length = healedVerb.length - 2; // No spaces
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
                            action->type = MELEE;
                            action->source.data = message.data;
                            action->source.length = i;
                            action->verb.data = message.data + i + 1; // Skip the space
                            action->verb.length = verbs[j].length - 2; // No spaces
                            action->target.data = action->verb.data + action->verb.length + 1; // Skip the space
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
                        action->target.length = message.data + i - action->target.data;
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
                    action->damage = toInt(damage);
                    break;
                }
            }
        }
    }
}

struct Action parseAction(struct String message)
{
    struct Action action = {0};
    {
        const struct String pointsOfDamage = CONST_STRING("points of damage.");
        const struct String pointOfDamage = CONST_STRING("point of damage.");
        if(endsWith(message, pointsOfDamage) || endsWith(message, pointOfDamage))
        {
            splitVerb(message, &action);
        }
    }
    return action;
}
