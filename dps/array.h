#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))

struct Array;

// Default-initalizer for Array
void Array_ctor(struct Array* this, size_t datumSize);
struct Array* Array_new(size_t datumSize);

struct Array
{
    size_t capacity;
    size_t size;
    size_t datumSize;
    uint8_t* data;
    // Functions
    void (*resize)(struct Array* this, size_t newCapacity);
    void (*clear)(struct Array* this);
    void (*push)(struct Array* this, void* datum);
    void* (*at)(struct Array* this, size_t pos);
    void (*dtor)(struct Array* this);
};

#endif // ARRAY_H

