#ifndef UTILITY_H
#define UTILITY_H

#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>

struct SimpleString
{
    char* data;
    size_t length;
};

// TODO: I'm not super happy with mixing and matching String, SimpleString, and
// `const char*` inputs. Would be nice to have a clever solution to this
// problem. I like the 'hold' stuff, but it gets a little onerous.
struct String;
void String_ctor    (struct String* this);
void String_ctorHold(struct String* this, char* data, size_t length);
void String_ctorCopy(struct String* this, const char* data, size_t length);
void String_ctorCopyString(struct String* this, const struct SimpleString* that);
struct String* String_new();
struct String
{
    struct SimpleString;
    bool ownsPtr;
    void (*hold)         (struct String* this, char* data, size_t length);
    void (*dup)          (struct String* this, const char* data, size_t length);
    void (*cpy)          (struct String* this, const char* data, size_t length);
    void (*cat)          (struct String* this, const char* data, size_t length);
    bool (*cmp)          (struct String* this, const char* data, size_t length);
    bool (*startsWith)   (struct String* this, const struct SimpleString* that);
    bool (*endsWith)     (struct String* this, const struct SimpleString* that);
    size_t (*find)       (struct String* this, const struct SimpleString* that, struct SimpleString* found);
    bool (*op_equal)     (struct String* this, const struct SimpleString* that);
    bool (*fromInt)      (struct String* this, int64_t value);
    int64_t (*toInt)     (struct String* this);
    void (*clear)        (struct String* this);
    struct SimpleString* (*to_SimpleString)(struct String* this);
    void (*dtor)         (struct String* this);
};
// Helper macro/function to make strings on the stack
#define SIMPLE_STRING(_x) { .data = _x, .length = strlen(_x)}
#define CONST_STRING(_x) _CONST_STRING(_x, strlen(_x))
static inline const struct String _CONST_STRING(const char* data, size_t length)
{
    // Very carefully cast const away from data, but assign it to a const structure
    // Does this do what I think it does?
    struct String tmp;
    String_ctorHold(&tmp, (char*)data, length);
    return tmp;
}

extern struct SimpleString g_empty;

#endif // UTILITY_H
