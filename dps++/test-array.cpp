#include "array.h"
#include "test.h"

void datum_dtor(void* datum)
{
    struct Array* array = (struct Array*)datum;
    array->dtor(array);
}

int testNestedArrays()
{
    int errors = 0;
    struct Array array;
    Array_ctor(&array, sizeof(struct Array));
    array.datum_dtor = datum_dtor;

    /* Create an array of arrays like this:
     * [
     *  [ ],
     *  [ 0 ],
     *  [ 0, 1 ],
     *  [ 0, 1, 2],
     *  ...
     */
    for(int count = 0; count < 10; count++)
    {
        // Create and fill the nested array
        struct Array nested;
        Array_ctor(&nested, sizeof(int));
        for(int value = 0; value < count; value++)
        {
            nested.push(&nested, &value);
        }
        // Push the nested array onto the array
        array.push(&array, &nested);
        // No nested.dtor
    }
    
    // Check array[0]
    {
        struct Array* subZero = array.at(&array, 0);
        test_uint_eq(subZero->size, 0);
        test_uint_eq(subZero->capacity, 0);
        test_uint_eq(subZero->datumSize, sizeof(int));
    }
    // Check array[2]
    {
        struct Array* subTwo = array.at(&array, 2);
        test_uint_eq(subTwo->size, 2);
        _test(errors, subTwo->capacity >= 2, true);
        test_uint_eq(subTwo->datumSize, sizeof(int));
        test_int_eq(*(int*)(subTwo->at(subTwo, 0)), 0);
        test_int_eq(*(int*)(subTwo->at(subTwo, 1)), 1);
    }
    // Check array[9]
    {
        struct Array* subNine = array.at(&array, 9);
        test_uint_eq(subNine->size, 9);
        _test(errors, subNine->capacity >= 9, true);
        test_uint_eq(subNine->datumSize, sizeof(int));
        for(int value = 0; value < 9; value++)
        {
            test_int_eq(*(int*)(subNine->at(subNine, value)), value);
        }
    }

    array.dtor(&array);
    return errors;
}

int testArray()
{
    int errors = 0;
    struct Array array;
    Array_ctor(&array, sizeof(int));

    array.resize(&array, 10);
    test_uint_eq(array.size, 0);
    test_uint_eq(array.capacity, 10);
    test_uint_eq(array.datumSize, sizeof(int));
    int newValue = 0;
    array.push(&array, &newValue);
    newValue++;
    array.push(&array, &newValue);
    newValue++;
    test_int_eq(array.size, 2);
    test_int_eq(array.capacity, 10);
    test_int_eq(*(int*)array.at(&array, 0), 0);
    test_int_eq(*(int*)array.at(&array, 1), 1);
    for(; newValue < 10; newValue++)
    {
        array.push(&array, &newValue);
    }
    test_uint_eq(array.size, 10);
    test_uint_eq(array.capacity, 10);
    test_int_eq(*(int*)array.at(&array, 8), 8);
    test_int_eq(*(int*)array.at(&array, 9), 9);
    array.push(&array, &newValue);
    newValue++;
    test_uint_eq(array.size, 11);
    test_uint_eq(array.capacity, 20);
    test_int_eq(*(int*)array.at(&array, 10), 10);
    for(; newValue < 1024; newValue++)
    {
        array.push(&array, &newValue);
    }
    test_uint_eq(array.size, 1024);
    test_uint_eq(array.capacity, 1024);

    array.resize(&array, 10);
    test_uint_eq(array.size, 10);
    test_uint_eq(array.capacity, 10);
    array.dtor(&array);
    return errors;
}

int main()
{
    int errors = 0;
    errors += testArray();
    errors += testNestedArrays();
    return errors;
}
