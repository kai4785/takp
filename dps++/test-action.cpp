#include "action.h"
#include "test.h"

#include <string_view>

using namespace std;
using namespace std::literals;

struct FakeAction
{
    Action::Type type;
    string_view source;
    string_view target;
    string_view verb;
    int64_t damage;
    string_view message;
};

int _validateAction(FakeAction action, const char* file, int line);

#define validateAction(_x) _validateAction(_x, __FILE__, __LINE__)

int _validateAction(FakeAction action, const char* file, int line)
{
    int errors = 0;
    Action newAction{action.message};
    if(action.type != newAction.type())
    {
        cerr << "[" << file << ":" << line << "]: Action type mismatch: " << action.type << " != " << newAction.type() << endl;
        cerr << "  :" << action.message << endl;
        errors++;
    }
    if(action.source != newAction.source())
    {
        cerr << "[" << file << ":" << line << "]: Action source mismatch: " << action.source << " != " << newAction.source() << endl;
        cerr << "  :" << action.message << endl;
        errors++;
    }
    if(action.target != newAction.target())
    {
        cerr << "[" << file << ":" << line << "]: Action target mismatch: " << action.target << " != " << newAction.target() << endl;
        cerr << "  :" << action.message << endl;
        errors++;
    }
    if(action.verb != newAction.verb())
    {
        cerr << "[" << file << ":" << line << "]: Action verb mismatch: " << action.verb << " != " << newAction.verb() << endl;
        cerr << "  :" << action.message << endl;
        errors++;
    }
    if(action.damage != newAction.damage())
    {
        cerr << "[" << file << ":" << line << "]: Action damage mismatch: " << action.damage << " != " << newAction.damage() << endl;
        cerr << "  :" << action.message << endl;
        errors++;
    }
    return errors;
}

int testActions()
{
    int errors = 0;

    FakeAction action;

    action.type = Action::HEAL;
    action.source = ""sv;
    action.target = "You"sv;
    action.verb = "have been healed"sv;
    action.damage = 2090;
    action.message = "You have been healed for 2090 points of damage.";
    errors += validateAction(action);

    action.type = Action::HEAL;
    action.source = ""sv;
    action.target = "You"sv;
    action.verb = "have been healed"sv;
    action.damage = 6;
    action.message = "You have been healed for 6 points of damage.";
    errors += validateAction(action);

    action.type = Action::MELEE;
    action.source = "You"sv;
    action.target = "a blizzard hunter"sv;
    action.verb = "slash"sv;
    action.damage = 39;
    action.message = "You slash a blizzard hunter for 39 points of damage.";
    errors += validateAction(action);

    action.type = Action::MAGIC;
    action.source = "Spell/DS(Total)"sv;
    action.target = "a blizzard hunter"sv;
    action.verb = "non-melee"sv;
    action.damage = 8;
    action.message = "a blizzard hunter was hit by non-melee for 8 points of damage.";
    errors += validateAction(action);

    action.type = Action::MELEE;
    action.source = "Foo`s warder"sv;
    action.target = "a giant lizard"sv;
    action.verb = "slashes"sv;
    action.damage = 33;
    action.message = "Foo`s warder slashes a giant lizard for 33 points of damage.";
    errors += validateAction(action);

    action.type = Action::MAGIC;
    action.source = "AoE Damage"sv;
    action.target = "You"sv;
    action.verb = "have taken"sv;
    action.damage = 300;
    action.message = "You have been lacerated.  You have taken 300 points of damage.";
    errors += validateAction(action);

    action.type = Action::MAGIC;
    action.source = "AoE Damage"sv;
    action.target = "You"sv;
    action.verb = "have taken"sv;
    action.damage = 1;
    action.message = "You have been lacerated.  You have taken 1 points of damage.";
    errors += validateAction(action);

    action.type = Action::MAGIC;
    action.source = "AoE Damage"sv;
    action.target = "You"sv;
    action.verb = "have taken"sv;
    action.damage = 130;
    action.message = "Your body combusts as the lava hits you.  You have taken 130 points of damage.";
    errors += validateAction(action);

    action.type = Action::MAGIC;
    action.source = "AoE Damage"sv;
    action.target = "You"sv;
    action.verb = "have taken"sv;
    action.damage = 1;
    action.message = "Your body combusts as the lava hits you.  You have taken 1 point of damage.";
    errors += validateAction(action);

    action.type = Action::DEATH;
    action.source = "You"sv;
    action.target = "Player1"sv;
    action.verb = "have slain"sv;
    action.damage = 0;
    action.message = "You have slain Player1!";
    errors += validateAction(action);

    action.type = Action::DEATH;
    action.source = "Player1"sv;
    action.target = "You"sv;
    action.verb = "have been slain by"sv;
    action.damage = 0;
    action.message = "You have been slain by Player1!";
    errors += validateAction(action);

    action.type = Action::DEATH;
    action.source = "Player2"sv;
    action.target = "Player1"sv;
    action.verb = "has been slain by"sv;
    action.damage = 0;
    action.message = "Player1 has been slain by Player2!";
    errors += validateAction(action);

    action.type = Action::DEATH;
    action.source = ""sv;
    action.target = "Player1"sv;
    action.verb = "died"sv;
    action.damage = 0;
    action.message = "Player1 died.";
    errors += validateAction(action);

    // TODO:
    // Critical hits
    // Crippling blows
    // Finishing blows
    // Critical blasts

    action.type = Action::MELEE;
    action.source = "Player1"sv;
    action.target = "Player2"sv;
    action.verb = "backstabs";
    action.damage = 10;
    action.message = "Player1 backstabs Player2 for 10 points of damage.";
    errors += validateAction(action);

    return errors;
}

int main()
{
    int errors = 0;
    errors += testActions();
    printf("Tests failed: %d\n", errors);
    return errors;
}
