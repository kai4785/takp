#include "action.h"
#include "battle.h"
#include "config.h"
#include "date.h"
#include "tail.h"

#include <string.h>
#include <stdio.h>

// TODO: Really?
 __attribute((constructor)) void configInstance(void)
{
    config.follow = false;
    config.history = true;
    config.me = NULL;
    config.logfile = NULL;
    config.since = 0;
    config.keepalive = 10;
    config.verbose = 0;
}

#define dateEquals(_x, _y) _dateEquals(_x, _y, __FILE__, __LINE__)

// TODO: Why can't this be const?
int _dateEquals(/*const*/ char* date, int64_t expected, const char* file, int line)
{
    int errors = 0;
    const struct String dateString =
    {
        .data = date,
        .length = strlen(date)
    };
    int64_t got = parseDate(dateString);
    if(got != expected)
    {
        fprintf(stderr, "Failed to parse date: %s:%d\n", file, line);
        fprintf(stderr, "   date:  [%d]%.*s\n", (int)dateString.length, (int)dateString.length, dateString.data);
        fprintf(stderr, "   value: %ld != %ld\n", got, expected);
        errors++;
    }
    return errors;
}

int testDates()
{
    int errors = 0;

    errors += dateEquals("Tue Mar 16 00:00:00 1999",           0LL);
    errors += dateEquals("Tue Mar 16 00:00:01 1999",           1LL);
    errors += dateEquals("Tue Mar 16 00:01:00 1999",          60LL);
    errors += dateEquals("Tue Mar 16 01:00:00 1999",        3600LL);
    errors += dateEquals("Tue Apr 16 00:00:00 1999",     2678400LL);
    errors += dateEquals("Tue Mar 16 00:00:00 2000",    31622400LL); // Leap year!
    errors += dateEquals("Wed Mar 06 22:09:37 2019",   630367777LL);
    errors += dateEquals("Wed Mar 06 22:10:46 2019",   630367846LL);

    return errors;
}

#define validateAction(_x) _validateAction(_x, __FILE__, __LINE__)

int _validateAction(const struct Action action, const char* file, int line)
{
    int errors = 0;
    struct String actionMessage = CONST_STRING((char*)action.message);
    struct Action newAction = parseAction(actionMessage);
    if(!action.type == newAction.type)
    {
        fprintf(stderr, "Action type mismatch: %d != %d\n", action.type, newAction.type);
        errors++;
    }
    if(!equalsTo(action.source, newAction.source))
    {
        fprintf(stderr, "Action source mismatch: %.*s != %.*s\n",
            (int)action.source.length, action.source.data,
            (int)newAction.source.length, newAction.source.data);
        errors++;
    }
    if(!equalsTo(action.target, newAction.target))
    {
        fprintf(stderr, "Action target mismatch: %.*s != %.*s\n",
            (int)action.target.length, action.target.data,
            (int)newAction.target.length, newAction.target.data);
        errors++;
    }
    if(!equalsTo(action.verb, newAction.verb))
    {
        fprintf(stderr, "Action verb mismatch: %.*s != %.*s\n",
            (int)action.verb.length, action.verb.data,
            (int)newAction.verb.length, newAction.verb.data);
        errors++;
    }
    if(action.damage != newAction.damage)
    {
        fprintf(stderr, "Action damage mismatch: %ld != %ld\n",
            action.damage,
            newAction.damage);
        errors++;
    }
    return errors;
}

int testActions()
{
    int errors = 0;

    struct Action action;

    action = (struct Action){
        .type = HEAL,
        .source = {0},
        .target = CONST_STRING("You"),
        .verb = CONST_STRING("have been healed"),
        .damage = 2090,
        .message = "You have been healed for 2090 points of damage."
    };
    errors += validateAction(action);

    action = (struct Action){
        .type = MELEE,
        .source = CONST_STRING("You"),
        .target = CONST_STRING("a blizzard hunter"),
        .verb = CONST_STRING("slash"),
        .damage = 39,
        .message = "You slash a blizzard hunter for 39 points of damage."
    };
    errors += validateAction(action);

    action = (struct Action){
        .type = MELEE,
        .source = {0},
        .target = CONST_STRING("a blizzard hunter"),
        .verb = CONST_STRING("non-melee"),
        .damage = 8,
        .message = "a blizzard hunter was hit by non-melee for 8 points of damage."
    };
    errors += validateAction(action);

    action = (struct Action){
        .type = MELEE,
        .source = CONST_STRING("Foo`s warder"),
        .target = CONST_STRING("a giant lizard"),
        .verb = CONST_STRING("slashes"),
        .damage = 33,
        .message = "Foo`s warder slashes a giant lizard for 33 points of damage."
    };
    errors += validateAction(action);

    return errors;
}

int main()
{
    int errors = 0;
    errors += testDates();
    errors += testActions();
    printf("Tests failed: %d\n", errors);
    return errors;
}
