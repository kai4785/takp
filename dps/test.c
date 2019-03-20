#include "array.h"
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
    config.me = (struct SimpleString)SIMPLE_STRING("Meriadoc");
    config.logfile = NULL;
    config.since = 0;
    config.keepAlive = 10;
    config.verbosity = 0;
    Battle_ctor(&battle);
}

#define dateEquals(_x, _y) _dateEquals(_x, _y, __FILE__, __LINE__)

// TODO: Why can't this be const?
int _dateEquals(/*const*/ char* date, int64_t expected, const char* file, int line)
{
    int errors = 0;
    const struct SimpleString dateString =
    {
        .data = date,
        .length = strlen(date)
    };
    int64_t got = parseDate(dateString);
    if(got != expected)
    {
        fprintf(stderr, "Failed to parse date: %s:%d\n", file, line);
        fprintf(stderr, "   date:  [%d]%.*s\n", (int)dateString.length, (int)dateString.length, dateString.data);
        fprintf(stderr, "   value: %jd != %jd\n", got, expected);
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
        fprintf(stderr, "[%s:%d]: Action source mismatch: %.*s != %.*s\n",
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
        fprintf(stderr, "[%s:%d]: Action damage mismatch: %jd != %jd\n",
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
    action.source.clear(&action.source);
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
    action.source.clear(&action.source);
    action.target = CONST_STRING("You");
    action.verb = CONST_STRING("have taken");
    action.damage = 300;
    action.message = "You have been lacerated.  You have taken 300 points of damage.";
    errors += validateAction(action);

    action.type = MAGIC;
    action.source.clear(&action.source);
    action.target = CONST_STRING("You");
    action.verb = CONST_STRING("have taken");
    action.damage = 1;
    action.message = "You have been lacerated.  You have taken 1 points of damage.";
    errors += validateAction(action);

    action.type = MAGIC;
    action.source.clear(&action.source);
    action.target = CONST_STRING("You");
    action.verb = CONST_STRING("have taken");
    action.damage = 130;
    action.message = "Your body combusts as the lava hits you.  You have taken 130 points of damage.";
    errors += validateAction(action);

    action.type = MAGIC;
    action.source.clear(&action.source);
    action.target = CONST_STRING("You");
    action.verb = CONST_STRING("have taken");
    action.damage = 1;
    action.message = "Your body combusts as the lava hits you.  You have taken 1 point of damage.";
    errors += validateAction(action);

    action.dtor(&action);
    return errors;
}

#define __test(_l, _r) \
if((_l) != (_r)) { \
    fprintf(stderr, "%s:%d: Comparison failed: (%lu == %lu)\n", __FILE__, __LINE__, (_l), (_r)); \
    errors++; \
}

int testArray()
{
    int errors = 0;
    {
        struct Array array;
        Array_ctor(&array, sizeof(int64_t));

        array.resize(&array, 10);
        __test(array.size, 0UL);
        __test(array.capacity, 10UL);
        __test(array.datumSize, sizeof(int64_t));
        int64_t newValue = 0;
        array.push(&array, &newValue);
        newValue++;
        array.push(&array, &newValue);
        newValue++;
        __test(array.size, 2UL);
        __test(array.capacity, 10UL);
        __test(*(int64_t*)array.at(&array, 0), 0UL);
        __test(*(int64_t*)array.at(&array, 1), 1UL);
        for(; newValue < 10; newValue++)
        {
            array.push(&array, &newValue);
        }
        __test(array.size, 10UL);
        __test(array.capacity, 10UL);
        __test(*(int64_t*)array.at(&array, 8), 8UL);
        __test(*(int64_t*)array.at(&array, 9), 9UL);
        array.push(&array, &newValue);
        newValue++;
        __test(array.size, 11UL);
        __test(array.capacity, 20UL);
        __test(*(int64_t*)array.at(&array, 10), 10UL);
        for(; newValue < 1024; newValue++)
        {
            array.push(&array, &newValue);
        }
        __test(array.size, 1024UL);
        __test(array.capacity, 1024UL);

        array.resize(&array, 10);
        __test(array.size, 10UL);
        __test(array.capacity, 10UL);
        array.dtor(&array);
    }
    return errors;
}

int main()
{
    int errors = 0;
    errors += testDates();
    errors += testActions();
    errors += testArray();
    printf("Tests failed: %d\n", errors);
    return errors;
}
