#include "array.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Member function declarations
void Array_push(struct Array* this, void* datum);
void Array_swap(struct Array* this, size_t lpos, size_t rpos);
void Array_resize(struct Array* this, size_t newCapacity);
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
    this->swap = &Array_swap;
    this->at = &Array_at;
    this->datum_dtor = NULL;
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

void Array_resize(struct Array* this, size_t newCapacity)
{
    // Easy, if we haven't allocated anything, allocate it now.
    if(!this->data && newCapacity > 0)
    {
        this->capacity = newCapacity;
        size_t allocBytes = this->capacity * this->datumSize;
        this->data = malloc(allocBytes);
        if(!this->data)
            bailout();

        memset(this->data, 0, allocBytes);
    }
    else
    {
        // Call the destructor for anything we're about to kick out
        if(this->size > newCapacity)
        {
            if(this->datum_dtor)
            {
                for(size_t i = newCapacity; i < this->size; i++)
                {
                    this->datum_dtor(this->data + (i * this->datumSize));
                }
            }
            this->size = newCapacity;
        }
        // Special case freeing the whole Array
        if(newCapacity == 0)
        {
            free(this->data);
            this->data = NULL;
            this->size = this->capacity = 0;
        }
        else
        {
            size_t allocBytes = newCapacity * this->datumSize;
            size_t oldBytes = this->capacity * this->datumSize;

            this->data = realloc(this->data, allocBytes);
            if(!this->data)
                bailout();

            if(this->capacity < newCapacity)
                memset(this->data + oldBytes, 0, allocBytes - oldBytes);

            this->capacity = newCapacity;
        }

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

void Array_swap(struct Array* this, size_t lpos, size_t rpos)
{
    uint8_t *lhs = this->data + (this->datumSize * lpos);
    uint8_t *rhs = this->data + (this->datumSize * rpos);
    uint8_t *tmp = malloc(this->datumSize);
    memcpy(tmp, lhs, this->datumSize);
    memcpy(lhs, rhs, this->datumSize);
    memcpy(rhs, tmp, this->datumSize);
    free(tmp);
}

void Array_dtor(struct Array* this)
{
    this->clear(this);
    this->data = NULL;
    this->size = this->capacity = 0;
}

void* Array_at(struct Array* this, size_t pos)
{
    if(pos >= this->size)
        return NULL;
    return this->data + (this->datumSize * pos);
}
