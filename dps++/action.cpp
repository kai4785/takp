#include "action.h"
#include "utility.h"
#include "config.h"

#include <array>
#include <string_view>

using namespace std;
using namespace std::literals;

enum {
    State0,
    State1,
    State2,
    State3,
};
void Action::parseDamage(string_view& message)
{
    string_view here;
    auto aeVerb = " have taken "sv;
    // First things first, find the AE damage, cause sometimes it says "hit". It's really easy to find.
    for(size_t i = message.size() - 1; i > aeVerb.size() + 1; i--)
    {
        // Look for the space before the aeVerb that comes before the damage value
        if(message[i - aeVerb.size()] == ' ')
        {
            // Make sure that the aeVerb is found immediately before the damage value
            size_t verbStart = i - aeVerb.size();
            here = message.substr(verbStart, aeVerb.size());
            // Look for the aeVerb, if it's not found, there's no aeVerb anywhere
            if(here == aeVerb)
            {
                auto damage = message.substr(i, message.size() - i );
                // This is a special AE type message; we just parse it and return, or break and continue;
                m_type = MAGIC;
                m_source = "AoE Damage"sv;
                auto& config = configInstance();
                m_target = config.me;
                m_verb = aeVerb.substr(1, aeVerb.size() - 2); // Trim leading/trailing whitespace
                m_damage = toInt(damage);
                return;
            }
            break;
        }
    }
    // TODO: Measure this, does it take a time to build this on the stack?
    std::array<string_view, 29> verbs{ {
        {" backstab "sv},
        {" backstabs "sv},
        {" bash "sv},
        {" bashes "sv},
        {" bites "sv},
        {" claw "sv},
        {" claws "sv},
        {" crush "sv},
        {" crushes "sv},
        {" gore "sv},
        {" gores "sv},
        {" hit "sv},
        {" hits "sv},
        {" kick "sv},
        {" kicks "sv},
        {" maul "sv},
        {" mauls "sv},
        {" pierce "sv},
        {" pierces "sv},
        {" punch "sv},
        {" punches "sv},
        {" rend "sv},
        {" rends "sv},
        {" slash "sv},
        {" slashes "sv},
        {" slice "sv},
        {" slices "sv},
        {" smash "sv},
        {" smashes "sv},
    }};
    auto nonMeleeVerb1 = " was hit by "sv;
    auto nonMeleeVerb2 = " were hit by "sv;
    auto healedVerb = " have been healed "sv;
    // 0 initial
    // 1 found melee verb, looking for target length
    // 2 found non-melee verb, looking for damage value
    //   found target length, looking for damage value
    int state = State0;
    for(size_t i = 0; i < message.size(); i++)
    {
        // If we found the start of a word
        if(message[i] == ' ')
        {
            here = message.substr(i, message.size() - i);
            if(state == State0)
            {
                if(startsWith(here, nonMeleeVerb1) || startsWith(here,nonMeleeVerb2))
                {
                    size_t length = (here[2] == 'a') ? nonMeleeVerb1.size() : nonMeleeVerb2.size();
                    m_type = MAGIC;
                    m_source = "Spell/DS(Total)"sv;
                    m_target = message.substr(0, i);
                    m_verb = message.substr(i + length, 9);
                    i += length;
                    state = State2;
                }
                else if(startsWith(here, healedVerb))
                {
                    m_type = HEAL;
                    m_source = ""sv;
                    m_target = message.substr(0, i);
                    m_verb = message.substr(i + 1, healedVerb.size() - 2); // No spaces
                    i += healedVerb.size() - 2;
                    state = State2;
                }
                else
                {
                    // TODO: Make this a std::Array
                    for(auto verb : verbs)
                    {
                        if(startsWith(here, verb))
                        {
                            // We found it!
                            m_type = MELEE;
                            m_source = message.substr(0, i);
                            m_verb = message.substr(i + 1, verb.size() - 2); // No spaces
                            m_target = message.substr(m_source.size() + m_verb.size() + 2); // Skip the spaces
                            // Still need to truncate target, but we won't know until the next pass
                            state = State1;
                            i += verb.size();
                            break;
                        }
                    }
                }
            }
            else if(state == State1 || state == State2)
            {
                // Now we're looking for the end of target, by searching for "for"
                auto _for_ = " for "sv;
                if(startsWith(here, _for_))
                {
                    if(state == State1)
                        m_target.remove_suffix(here.size());
                    auto damage = message.substr(i + _for_.size(), message.size() - i - _for_.size());
                    m_damage = toInt(damage);
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

void Action::parse(const string_view& _message)
{
    string_view message = _message;
    if(message.back() == '.')
    {
        auto pointsOfDamage = " points of damage."sv;
        auto pointOfDamage =  " point of damage."sv;
        auto died = " died."sv;
        if(endsWith(message, pointsOfDamage))
        {
            message.remove_suffix(pointsOfDamage.size());
            parseDamage(message);
        }
        else if(endsWith(message, pointOfDamage))
        {
            message.remove_suffix(pointOfDamage.size());
            parseDamage(message);
        }
        else if(endsWith(message, died))
        {
            m_type = DEATH;
            m_target = message.substr(0, message.size() - died.size());
            m_verb = died.substr(1, died.size() - 2);
        }
    }
    else if(message.back() == ')')
    {
        auto critical_hit   = " Scores a critical hit!("sv;
        auto crippling_blow = " lands a Crippling Blow!("sv;
        auto holy_blade     = "'s holy blade cleanses his target!("sv;
        auto found = ""sv;
        size_t pos = 0;
        if(string_view::npos != (pos = message.find(critical_hit)))
        {
            found = message.substr(pos, critical_hit.size());
            m_type = CRIT;
        }
        else if(string_view::npos != (pos = message.find(crippling_blow)))
        {
            found = message.substr(pos, crippling_blow.size());
            m_type = CRIP;
        }
        else if(string_view::npos != (pos = message.find(holy_blade)))
        {
            found = message.substr(pos, holy_blade.size());
            m_type = HOLYBLADE;
        }
        if(m_type != UNKNOWN)
        {
            m_source = message.substr(0, pos);
            size_t damageStart = m_source.size() + found.size();
            size_t damageSize = message.size() - 1 - damageStart;
            auto damage = message.substr(damageStart, damageSize);
            m_damage = toInt(damage);
        }
    }
    else if(message.back() == '!')
    {
        auto have_slain         = " have slain "sv;
        auto has_been_slain_by  = " has been slain by "sv;
        auto have_been_slain_by = " have been slain by "sv;
        auto found = ""sv;
        size_t pos = 0;
        if(string_view::npos != (pos = message.find(have_slain)))
        {
            found = message.substr(pos, have_slain.size());
            m_type = DEATH;
            m_source = message.substr(0, pos);
            m_target = message.substr(m_source.size() + found.size(), message.size() - 1 - m_source.size() - found.size());
            m_verb = found.substr(1, found.size() - 2);
        }
        else if(
            (string_view::npos != (pos = message.find(has_been_slain_by)) ||
            (string_view::npos != (pos = message.find(have_been_slain_by)))))
        {
            auto found_size = message[pos + 3] == 's' ? has_been_slain_by.size() : have_been_slain_by.size();
            found = message.substr(pos, found_size);
            m_type = DEATH;
            m_target = message.substr(0, pos);
            m_source = message.substr(m_target.size() + found.size(), message.size() - 1 - m_target.size() - found.size());
            m_verb = found.substr(1, found.size() - 2);
        }
    }
}
