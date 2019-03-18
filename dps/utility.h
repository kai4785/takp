#ifndef UTILITY_H
#define UTILITY_H

#include "array.h"

#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>

struct String;
void String_ctor    (struct String* this);
void String_ctorHold(struct String* this, char* data, size_t length);
void String_ctorDup (struct String* this, char* data, size_t length);
void String_ctorCopy(struct String* this, struct String* that);
struct String* String_new();
struct String
{
    char* data;
    size_t length;
    bool ownsPtr;
    void (*hold)      (struct String* this, char* data, size_t length);
    void (*dup)       (struct String* this, char* data, size_t length);
    void (*copy)      (struct String* this, struct String* that);
    bool (*startsWith)(struct String* this, struct String* that);
    bool (*endsWith)  (struct String* this, struct String* that);
    bool (*op_equal)  (struct String* this, struct String* that);
    int64_t (*toInt)  (struct String* this);
    void (*clear)     (struct String* this);
    void (*dtor)      (struct String* this);
};
// Helper macro/function to make strings on the stack
#define CONST_STRING(_x) _CONST_STRING(_x, strlen(_x))
static inline struct String _CONST_STRING(char* data, size_t length)
{
    struct String tmp;
    String_ctorHold(&tmp, data, length);
    return tmp;
}

bool equalsTo(const struct String string, const struct String end);
bool startsWith(const struct String string, const struct String end);
bool endsWith(const struct String string, const struct String end);
int64_t toInt(const struct String string);

#endif // UTILITY_H
