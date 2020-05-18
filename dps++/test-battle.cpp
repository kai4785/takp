#include "battle.h"
#include "test.h"

#include <string_view>

using namespace std;
using namespace std::literals::string_view_literals;

int main()
{
    int errors = 0;
    int64_t now = 0;
    auto battle = battleInstance();
    // Add a Trivial melee
    Action action("Someone hits Someone Else for 1 point of damage."sv);

    battle->melee(now, action);

    // Print the report for debugging purposes
    battle->report();

    // Grab the fight
    test_uint_eq(battle->m_fight.size(), 1);
    auto& fight = battle->m_fight.at(0);

    // Make sure the fight matches the action(s) so far
    test_uint_eq(fight.seconds(), 1);
    test_uint_eq(fight.m_melee.hits, 1);
    test_uint_eq(fight.m_melee.damage, 1);
    test_uint_eq(dps(fight.m_melee, fight.seconds()), 1);
    test_uint_eq(hps(fight.m_melee, fight.seconds()), 1);
    test_uint_eq(dph(fight.m_melee), 1);

    // Hit them again, inside the same second
    battle->melee(now, action);
    battle->report();

    // Should be the same fight
    test_uint_eq(fight.seconds(), 1);
    test_uint_eq(fight.m_melee.hits, 2);
    test_uint_eq(fight.m_melee.damage, 2);
    test_uint_eq(dps(fight.m_melee, fight.seconds()), 2);
    test_uint_eq(hps(fight.m_melee, fight.seconds()), 2);
    test_uint_eq(dph(fight.m_melee), 1);

    // New fight, Someone Else hits back!
    action.parse("Someone Else hits Someone for 1 point of damage."sv);
    battle->melee(now, action);
    battle->report();

    // Grab the new fight
    test_uint_eq(battle->m_fight.size(), 2);
    fight = battle->m_fight.at(1);

    // Should look like the first fight
    test_uint_eq(fight.seconds(), 1);
    test_uint_eq(fight.m_melee.hits, 1);
    test_uint_eq(fight.m_melee.damage, 1);
    test_uint_eq(dps(fight.m_melee, fight.seconds()), 1);
    test_uint_eq(hps(fight.m_melee, fight.seconds()), 1);
    test_uint_eq(dph(fight.m_melee), 1);

    // TODO:
    // Backstabs
    // Critical hits
    // Crippling blows
    // Finishing blows

    return errors;
}
