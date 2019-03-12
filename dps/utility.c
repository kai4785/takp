#include "utility.h"

#include <string.h>
#include <stdlib.h>

bool equalsTo(const struct String lhs, struct String rhs)
{
    if(lhs.length != rhs.length)
        return false;
    return (strncmp(lhs.data, rhs.data, rhs.length) == 0);
}

bool startsWith(const struct String lhs, struct String rhs)
{
    if(lhs.length < rhs.length)
        return false;
    return (strncmp(lhs.data, rhs.data, rhs.length) == 0);
}

bool endsWith(const struct String lhs, struct String rhs)
{
    if(lhs.length < rhs.length)
        return false;
    return (strncmp(lhs.data + lhs.length - rhs.length, rhs.data, rhs.length) == 0);
}

int64_t toInt(const struct String string)
{
    int64_t value = 0;
    int64_t base = 1;
    for(ssize_t i = string.length - 1; i >= 0; i--)
    {
        value += (string.data[i] - '0') * base;
        base *= 10;
    }
    return value;
}

struct Words split(struct String message)
{
    struct Words words = {0};
    for(size_t i = 0; i < message.length; i++)
    {
        if(message.data[i] == ' ')
            words.length++;
    }
    words.length++;

    size_t allocSize = sizeof(struct String) * words.length;
    words.data = malloc(allocSize);
    memset(words.data, '\0', allocSize);
    size_t start = 0, end = 0, j = 0;
    for(size_t i = 0; i < message.length; i++)
    {
        if(message.data[i] == ' ')
        {
            words.data[j].data = strndup(&message.data[start], i - start);
            j++;
            end = i;
            start = i+1;
        }
        if(i + 1 == message.length)
        {
            words.data[j].data = strndup(&message.data[start], i - start + 1);
        }
    }
    return words;
}

void freeWords(struct Words* words)
{
    if(!words)
        return;
    for(size_t i = 0; i < words->length; i++)
    {
        free(words->data[i].data);
    }
    free(words->data);
    words->data = NULL;
    words->length = 0;
}
