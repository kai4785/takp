#ifndef UTILITY_H
#define UTILITY_H

#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>

#define CONST_STRING(x) {.data = x, .length = strlen(x),}
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))
struct String
{
    char* data;
    size_t length;
};

struct Words
{
    struct String* data;
    size_t length;
};

bool startsWith(const struct String string, const struct String end);
bool endsWith(const struct String string, const struct String end);
int64_t toInt(const struct String string);

// Allocates a Words struct. Free with freeWords
struct Words split(const struct String message);
// Frees and zeros out words
void freeWords(struct Words* words);

#endif // UTILITY_H
