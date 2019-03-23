#include "array.h"
#include "test.h"

#define __test(_l, _r) \
if((_l) != (_r)) { \
    fprintf(stderr, "%s:%d: Comparison failed: (%lu == %lu)\n", __FILE__, __LINE__, (_l), (_r)); \
    errors++; \
}

int testArray()
{
    int errors = 0;
    {
        struct Array array;
        Array_ctor(&array, sizeof(unsigned long));

        array.resize(&array, 10);
        __test(array.size, 0UL);
        __test(array.capacity, 10UL);
        __test(array.datumSize, sizeof(unsigned long));
        unsigned long newValue = 0;
        array.push(&array, &newValue);
        newValue++;
        array.push(&array, &newValue);
        newValue++;
        __test(array.size, 2UL);
        __test(array.capacity, 10UL);
        __test(*(unsigned long*)array.at(&array, 0), 0UL);
        __test(*(unsigned long*)array.at(&array, 1), 1UL);
        for(; newValue < 10; newValue++)
        {
            array.push(&array, &newValue);
        }
        __test(array.size, 10UL);
        __test(array.capacity, 10UL);
        __test(*(unsigned long*)array.at(&array, 8), 8UL);
        __test(*(unsigned long*)array.at(&array, 9), 9UL);
        array.push(&array, &newValue);
        newValue++;
        __test(array.size, 11UL);
        __test(array.capacity, 20UL);
        __test(*(unsigned long*)array.at(&array, 10), 10UL);
        for(; newValue < 1024; newValue++)
        {
            array.push(&array, &newValue);
        }
        __test(array.size, 1024UL);
        __test(array.capacity, 1024UL);

        array.resize(&array, 10);
        __test(array.size, 10UL);
        __test(array.capacity, 10UL);
        array.dtor(&array);
    }
    return errors;
}

int main()
{
    int errors = 0;
    errors += testArray();
    return errors;
}
