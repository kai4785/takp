#include "array.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Member function declarations
void Array_push(struct Array* this, void* datum);
void Array_resize(struct Array* this, size_t count);
void Array_clear(struct Array* this);
void* Array_at(struct Array* this, size_t pos);
void Array_dtor(struct Array* this);

// Constructors
void Array_ctor(struct Array* this, size_t datumSize)
{
    *this = (struct Array){0};
    this->datumSize = datumSize;
    this->resize = &Array_resize;
    this->clear = &Array_clear;
    this->push = &Array_push;
    this->at = &Array_at;
    this->dtor = &Array_dtor;
}

struct Array* Array_new(size_t datumSize)
{
    struct Array *this = malloc(sizeof(struct Array));
    Array_ctor(this, datumSize);
    return this;
}

// Member function implementations

// Just bail out when memory is exhausted
void bailout()
{
    fprintf(stderr, "Could not allocate memory. Let's just bail.");
    abort();
}

size_t nextCapacity(size_t capacity)
{
    if(capacity == 0)
    {
        capacity = 16;
    }
    else if(capacity < 1024)
    {
        if(capacity > 512)
        {
            capacity = 1024;
        }
        else
        {
            capacity += capacity;
        }
    }
    else
    {
        capacity += 1024;
    }
    return capacity;
}

// Function Pointer methods

void Array_resize(struct Array* this, size_t count)
{
    if(!this->data)
    {
        this->capacity = count;
        size_t allocBytes = this->capacity * this->datumSize;
        this->data = malloc(allocBytes);
        if(!this->data)
            bailout();

        memset(this->data, 0, allocBytes);
    }
    else
    {
        size_t oldCapacity = this->capacity;
        size_t oldBytes = oldCapacity * this->datumSize;
        this->capacity = count;

        size_t allocBytes = this->capacity * this->datumSize;
        this->data = realloc(this->data, allocBytes);
        if(!this->data)
            bailout();

        if(oldCapacity < count)
            memset(this->data + oldBytes, 0, allocBytes - oldBytes);

        if(this->size > count)
            this->size = count;
    }
}

void Array_clear(struct Array* this)
{
    Array_resize(this, 0);
}

void Array_push(struct Array* this, void* datum)
{
    if(this->size == this->capacity)
    {
        Array_resize(this, nextCapacity(this->capacity));
    }
    memcpy(this->data + (this->datumSize * this->size), datum, this->datumSize);
    this->size++;
}

void Array_dtor(struct Array* this)
{
    free(this->data);
    this->data = NULL;
}

void* Array_at(struct Array* this, size_t pos)
{
    if(pos >= this->size)
        return NULL;
    return this->data + (this->datumSize * pos);
}
