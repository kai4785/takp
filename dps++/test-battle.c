#include "battle.h"
#include "test.h"

int main()
{
    int errors = 0;
    struct Battle* battle = battleInstance();
    struct Action action;
    Action_ctor(&action);
    int64_t now = 0;

    // Add a Trivial melee
    action.type = MELEE;
    action.source = CONST_STRING("Someone");
    action.target = CONST_STRING("Someone Else");
    action.verb = CONST_STRING("hits");
    action.damage = 1;
    battle->melee(battle, now, &action);

    // Print the report for debugging purposes
    battle->report(battle);

    // Grab the fight
    test_uint_eq(battle->m_fight.size, 1);
    struct Fight* fight = battle->m_fight.at(&battle->m_fight, 0);

    // Make sure the fight matches the action(s) so far
    test_uint_eq(fight->seconds(fight), 1);
    test_uint_eq(fight->m_melee.hits, 1);
    test_uint_eq(fight->m_melee.damage, 1);
    test_uint_eq(dps(fight->m_melee, fight->seconds(fight)), 1);
    test_uint_eq(hps(fight->m_melee, fight->seconds(fight)), 1);
    test_uint_eq(dph(fight->m_melee), 1);

    // Hit them again, inside the same second
    battle->melee(battle, now, &action);
    battle->report(battle);

    // Should be the same fight
    test_uint_eq(fight->seconds(fight), 1);
    test_uint_eq(fight->m_melee.hits, 2);
    test_uint_eq(fight->m_melee.damage, 2);
    test_uint_eq(dps(fight->m_melee, fight->seconds(fight)), 2);
    test_uint_eq(hps(fight->m_melee, fight->seconds(fight)), 2);
    test_uint_eq(dph(fight->m_melee), 1);

    // New fight, Someone Else hits back!
    action.type = MELEE;
    action.source = CONST_STRING("Someone Else");
    action.target = CONST_STRING("Someone");
    action.verb = CONST_STRING("hits");
    action.damage = 1;
    battle->melee(battle, now, &action);
    battle->report(battle);

    // Grab the new fight
    test_uint_eq(battle->m_fight.size, 2);
    fight = battle->m_fight.at(&battle->m_fight, 1);

    // Should look like the first fight
    test_uint_eq(fight->seconds(fight), 1);
    test_uint_eq(fight->m_melee.hits, 1);
    test_uint_eq(fight->m_melee.damage, 1);
    test_uint_eq(dps(fight->m_melee, fight->seconds(fight)), 1);
    test_uint_eq(hps(fight->m_melee, fight->seconds(fight)), 1);
    test_uint_eq(dph(fight->m_melee), 1);

    // TODO:
    // Backstabs
    // Critical hits
    // Crippling blows
    // Finishing blows

    return errors;
}
