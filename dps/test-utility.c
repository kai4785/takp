#include "utility.h"
#include "test.h"

#include <stdbool.h>

int cmp_String(struct String left, struct String right, bool equals)
{
    return (equals != (left.op_equal(&left, right.to_SimpleString(&right))));
}

void print_String(struct String value)
{
    fprintf(stderr, "(%zu)'%.*s'", value.length, (int)value.length, value.data);
}

#define test_eq(_x, _y) _test_eq(errors, cmp_String, print_String, _x, _y, true)
#define test_neq(_x, _y) _test_eq(errors, cmp_String, print_String, _x, _y, false)

#define test(_expr) _test(errors, _expr, true);
#define test_false(_expr) _test(errors, _expr, false);

int main()
{
    int errors = 0;
    // Constructor/Destructor
    {
        // Default constructor.
        {
            struct String astring;
            String_ctor(&astring);
            test(astring.dtor != NULL);
            astring.dtor(&astring);
        }
    }
    // Basic equality
    {
        struct String left = CONST_STRING("left");
        struct String right = CONST_STRING("right");
        test_eq(left, left);
        test_neq(left, right);
        left.dtor(&left);
        right.dtor(&right);
    }
    // Search for matches
    {
        struct String fox = CONST_STRING("the quick brown fox jumped over the lazy dog");
        struct SimpleString theStart = SIMPLE_STRING("the quick brown fox");
        struct SimpleString theEnd = SIMPLE_STRING("the lazy dog");
        struct SimpleString theMiddle = SIMPLE_STRING("brown fox jumped");
        _test(errors, fox.startsWith(&fox, &theStart), true);
        _test(errors, fox.endsWith(&fox, &theEnd), true);
        _test(errors, fox.startsWith(&fox, &theEnd), false);
        _test(errors, fox.endsWith(&fox, &theStart), false);
        struct SimpleString found = {0};
        _test(errors, fox.find(&fox, &theMiddle, &found), true);
        _test(errors, (found.length == theMiddle.length), true);
        _test(errors, (found.data == fox.data + 10), true);
        fox.dtor(&fox);
    }
    // Integer conversion
    {
        int64_t value = 0;
        char buf[10] = {'0'}; // Up to 10 Digits
        struct String string = {0};
        String_ctorHold(&string, buf, 1); // Check 1 Digit
        _test(errors, string.toInt(&string) == value, true);

        value = 9;
        struct String nine = CONST_STRING("9");
        _test(errors, string.fromInt(&string, value), true);
        test_eq(string, nine);
        _test(errors, string.toInt(&string) == value, true);

        value = 10;
        struct String ten = CONST_STRING("10");
        // Length == 1, so not enough digits
        _test(errors, string.fromInt(&string, value), false);
        string.length = 2;
        _test(errors, string.fromInt(&string, value), true);
        test_eq(string, ten);
        _test(errors, string.toInt(&string) == value, true);

        value = 1234;
        struct String onetwothreefour = CONST_STRING("1234");
        string.length = onetwothreefour.length;
        _test(errors, string.fromInt(&string, value), true);
        test_eq(string, onetwothreefour);
        _test(errors, string.toInt(&string) == value, true);
    }
    if(errors)
        fprintf(stderr, "Errors: %d\n", errors);
    return errors;
}
