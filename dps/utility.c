#include "utility.h"
#include "array.h"

#include <string.h>
#include <stdlib.h>

// Member function declarations
void String_hold      (struct String* this, char* data, size_t length);
void String_dup       (struct String* this, char* data, size_t length);
void String_copy      (struct String* this, struct String* that);
bool String_startsWith(struct String* this, struct String* that);
bool String_endsWith  (struct String* this, struct String* that);
bool String_op_equal  (struct String* this, struct String* that);
int64_t String_toInt  (struct String* this);
void String_clear     (struct String* this);
void String_dtor      (struct String* this);

// Constructors
void String_ctor(struct String* this)
{
    *this = (struct String){
        .data = NULL,
        .length = 0,
        .ownsPtr = false,
        .hold = &String_hold,
        .dup = &String_dup,
        .copy = &String_copy,
        .startsWith = &String_startsWith,
        .endsWith = &String_endsWith,
        .op_equal = &String_op_equal,
        .toInt = &String_toInt,
        .clear = &String_clear,
        .dtor = &String_dtor,
    };
}

void String_ctorHold(struct String* this, char* data, size_t length)
{
    String_ctor(this);
    this->data = data;
    this->length = length;
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

// Member function implementations
void String_hold(struct String* this, char* data, size_t length)
{
    this->clear(this);
    this->data = data;
    this->length = length;
    this->ownsPtr = false;
}

void String_dup(struct String* this, char* data, size_t length)
{
    this->clear(this);
    this->data = strndup(data, length);
    this->length = length;
    this->ownsPtr = true;
}

void String_copy(struct String* this, struct String* that)
{
    this->clear(this);
    this->data = strndup(that->data, that->length);
    this->length = that->length;
    this->ownsPtr = true;
}

bool String_startsWith(struct String* this, struct String* that)
{
    if(this->length < that->length)
        return false;
    return (strncmp(this->data, that->data, that->length) == 0);
}

bool String_endsWith(struct String* this, struct String* that)
{
    if(this->length < that->length)
        return false;
    return (strncmp(this->data + this->length - that->length, that->data, that->length) == 0);
}

bool String_op_equal(struct String* this, struct String* that)
{
    if(this->length != that->length)
        return false;
    return (strncmp(this->data, that->data, that->length) == 0);
}

int64_t String_toInt(struct String* this)
{
    int64_t value = 0;
    int64_t base = 1;
    for(ssize_t i = this->length - 1; i >= 0; i--)
    {
        value += (this->data[i] - '0') * base;
        base *= 10;
    }
    return value;
}

void String_clear(struct String* this)
{
    if(this->ownsPtr)
    {
        free(this->data);
    }
    this->data = NULL;
    this->length = 0;
    this->ownsPtr = false;
}

void String_dtor(struct String* this)
{
    this->clear(this);
}
