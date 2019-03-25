#include "action.h"
#include "test.h"

#define validateAction(_x) _validateAction(_x, __FILE__, __LINE__)

int _validateAction(struct Action action, const char* file, int line)
{
    int errors = 0;
    struct String actionMessage;
    String_ctorHold(&actionMessage, (char*)action.message, strlen(action.message));
    struct Action newAction;
    Action_ctor(&newAction);
    newAction.parse(&newAction, actionMessage);
    if(!(action.type == newAction.type))
    {
        fprintf(stderr, "[%s:%d]: Action type mismatch: %d != %d\n", file, line, action.type, newAction.type);
        fprintf(stderr, "  :%s\n", action.message);
        errors++;
    }
    if(!action.source.op_equal(&action.source, newAction.source.to_SimpleString(&newAction.source)))
    {
        fprintf(stderr, "[%s:%d]: Action source mismatch: '%.*s' != '%.*s'\n",
            file, line,
            (int)action.source.length, action.source.data,
            (int)newAction.source.length, newAction.source.data);
        fprintf(stderr, "  :%s\n", action.message);
        errors++;
    }
    if(!action.source.op_equal(&action.target, newAction.target.to_SimpleString(&newAction.target)))
    {
        fprintf(stderr, "[%s:%d]: Action target mismatch: %.*s != %.*s\n",
            file, line,
            (int)action.target.length, action.target.data,
            (int)newAction.target.length, newAction.target.data);
        fprintf(stderr, "  :%s\n", action.message);
        errors++;
    }
    if(!action.source.op_equal(&action.verb, newAction.verb.to_SimpleString(&newAction.verb)))
    {
        fprintf(stderr, "[%s:%d]: Action verb mismatch: %.*s != %.*s\n",
            file, line,
            (int)action.verb.length, action.verb.data,
            (int)newAction.verb.length, newAction.verb.data);
        fprintf(stderr, "  :%s\n", action.message);
        errors++;
    }
    if(action.damage != newAction.damage)
    {
        fprintf(stderr, "[%s:%d]: Action damage mismatch: %"PRId64" != %"PRId64"\n",
            file, line,
            action.damage,
            newAction.damage);
        fprintf(stderr, "  :%s\n", action.message);
        errors++;
    }
    newAction.dtor(&newAction);
    return errors;
}

int testActions()
{
    int errors = 0;

    struct Action action;
    Action_ctor(&action);

    action.type = HEAL;
    action.source.clear(&action.source);
    action.target = CONST_STRING("You");
    action.verb = CONST_STRING("have been healed");
    action.damage = 2090;
    action.message = "You have been healed for 2090 points of damage.";
    errors += validateAction(action);

    action.type = MELEE;
    action.source = CONST_STRING("You");
    action.target = CONST_STRING("a blizzard hunter");
    action.verb = CONST_STRING("slash");
    action.damage = 39;
    action.message = "You slash a blizzard hunter for 39 points of damage.";
    errors += validateAction(action);

    action.type = MAGIC;
    action.source = CONST_STRING("Spell/DS(Total)");
    action.target = CONST_STRING("a blizzard hunter");
    action.verb = CONST_STRING("non-melee");
    action.damage = 8;
    action.message = "a blizzard hunter was hit by non-melee for 8 points of damage.";
    errors += validateAction(action);

    action.type = MELEE;
    action.source = CONST_STRING("Foo`s warder");
    action.target = CONST_STRING("a giant lizard");
    action.verb = CONST_STRING("slashes");
    action.damage = 33;
    action.message = "Foo`s warder slashes a giant lizard for 33 points of damage.";
    errors += validateAction(action);

    action.type = MAGIC;
    action.source = CONST_STRING("Spell/DS(Total)");
    action.target = CONST_STRING("You");
    action.verb = CONST_STRING("have taken");
    action.damage = 300;
    action.message = "You have been lacerated.  You have taken 300 points of damage.";
    errors += validateAction(action);

    action.type = MAGIC;
    action.source = CONST_STRING("Spell/DS(Total)");
    action.target = CONST_STRING("You");
    action.verb = CONST_STRING("have taken");
    action.damage = 1;
    action.message = "You have been lacerated.  You have taken 1 points of damage.";
    errors += validateAction(action);

    action.type = MAGIC;
    action.source = CONST_STRING("Spell/DS(Total)");
    action.target = CONST_STRING("You");
    action.verb = CONST_STRING("have taken");
    action.damage = 130;
    action.message = "Your body combusts as the lava hits you.  You have taken 130 points of damage.";
    errors += validateAction(action);

    action.type = MAGIC;
    action.source = CONST_STRING("Spell/DS(Total)");
    action.target = CONST_STRING("You");
    action.verb = CONST_STRING("have taken");
    action.damage = 1;
    action.message = "Your body combusts as the lava hits you.  You have taken 1 point of damage.";
    errors += validateAction(action);

    // TODO: Handle finishing blows
    action.dtor(&action);
    return errors;
}

int main()
{
    int errors = 0;
    errors += testActions();
    printf("Tests failed: %d\n", errors);
    return errors;
}
