#ifndef ACTION_H
#define ACTION_H

#include "utility.h"

#include <stddef.h>
#include <inttypes.h>

enum ActionType
{
    UNKNOWN,
    CHAT,
    MELEE,
    MAGIC,
    HEAL,
    CRIT,
    CRIPP,
    DEATH,
    ZONE
};

struct Action;
void Action_ctor(struct Action* action);
struct Action* Action_new();
struct Action
{
    enum ActionType type;
    struct String source;
    struct String target;
    struct String verb;
    int64_t damage;
    const char* message;
    void(*parse) (struct Action* this, struct String message);
    void(*dtor) (struct Action* this);
};

struct Action parseAction(struct String message);

#endif // ACTION_H
