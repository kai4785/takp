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

struct Action
{
    enum ActionType type;
    struct String source;
    struct String target;
    struct String verb;
    int64_t damage;
    const char* message;
};

struct Action parseAction(struct String message);

#endif // ACTION_H
