#include "utility.h"
#include "array.h"

#include <string.h>
#include <stdlib.h>

struct SimpleString g_empty = { .data = "", .length = 0 };

// Member function declarations
void String_hold      (struct String* this, char* data, size_t length);
void String_dup       (struct String* this, char* data, size_t length);
void String_cpy       (struct String* this, char* data, size_t length);
bool String_cmp       (struct String* this, char* data, size_t length);
bool String_startsWith(struct String* this, struct SimpleString* that);
bool String_endsWith  (struct String* this, struct SimpleString* that);
bool String_find      (struct String* this, struct SimpleString* that, struct SimpleString* found);
bool String_op_equal  (struct String* this, struct SimpleString* that);
int64_t String_toInt  (struct String* this);
bool String_fromInt   (struct String* this, int64_t value);
void String_clear     (struct String* this);
struct SimpleString*  String_to_SimpleString(struct String* this);
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
        .cpy = &String_cpy,
        .cmp = &String_cmp,
        .startsWith = &String_startsWith,
        .endsWith = &String_endsWith,
        .find = &String_find,
        .op_equal = &String_op_equal,
        .toInt = &String_toInt,
        .fromInt = &String_fromInt,
        .clear = &String_clear,
        .to_SimpleString = &String_to_SimpleString,
        .dtor = &String_dtor,
    };
}

void String_ctorHold(struct String* this, char* data, size_t length)
{
    String_ctor(this);
    this->data = data;
    this->length = length;
}

void String_ctorCopy(struct String* this, struct SimpleString* that)
{
    String_ctor(this);
    this->dup(this, that->data, that->length);
}

// Member function implementations
bool String_ncmp(const char* lhs, const char* rhs, size_t length)
{
    const char* end = lhs + length;
    while(lhs < end)
    {
        if(*lhs++ != *rhs++)
            return false;
    }
    return true;
}

void String_ncpy(char* dest, const char* src, size_t length)
{
    const char* end = dest + length;
    while(dest < end)
    {
        *dest++ = *src++;
    }
}

void String_hold(struct String* this, char* data, size_t length)
{
    if(this->ownsPtr)
    {
        free(this->data);
    }
    this->data = data;
    this->length = length;
    this->ownsPtr = false;
}

void String_dup(struct String* this, char* data, size_t length)
{
    if(this->ownsPtr)
    {
        free(this->data);
    }
    this->data = malloc(length);
    this->ownsPtr = true;
    this->cpy(this, data, length);
}

void String_cpy(struct String* this, char* data, size_t length)
{
    String_ncpy(this->data, data, length);
    this->length = length;
}

bool String_cmp(struct String* this, char* data, size_t length)
{
    return String_ncmp(this->data, data, length);
}

bool String_startsWith(struct String* this, struct SimpleString* that)
{
    if(this->length < that->length)
        return false;
    return String_ncmp(this->data, that->data, that->length);
}

bool String_endsWith(struct String* this, struct SimpleString* that)
{
    if(this->length < that->length)
        return false;
    return String_ncmp(this->data + this->length - that->length, that->data, that->length);
}

bool String_find(struct String* this, struct SimpleString* that, struct SimpleString* found)
{
    if(this->length < that->length)
        return false;
    for(size_t i = 0; i < this->length - that->length; i++)
    {
        if(this->data[i] == that->data[0] && String_ncmp(this->data + i, that->data, that->length))
        {
            *found = (struct SimpleString){ .data = this->data + i, .length = that->length };
            return true;
        }
    }
    return false;
}

bool String_op_equal(struct String* this, struct SimpleString* that)
{
    if(this->length != that->length)
        return false;
    return String_ncmp(this->data, that->data, that->length);
}

bool String_fromInt(struct String* this, int64_t value)
{
    char* here = this->data + this->length - 1;
    int64_t base = 10;
    for(size_t i = 0; i < this->length; i++)
    {
        *(here--) = (value % base) + '0';
        value /= 10;
    }
    return (value == 0);
}

int64_t String_toInt(struct String* this)
{
    int64_t value = 0;
    int64_t base = 1;
    char* here = this->data + this->length - 1;
    for(size_t i = 0; i < this->length; i++)
    {
        value += (*(here--) - '0') * base;
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

struct SimpleString*  String_to_SimpleString(struct String* this)
{
    return (struct SimpleString*)this;
}

void String_dtor(struct String* this)
{
    this->clear(this);
}
