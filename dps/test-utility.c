#include "utility.h"
#include "test.h"

#include <stdbool.h>

int cmp_String(struct String left, struct String right, bool equals)
{
    return (equals != (left.op_equal(&left, right.to_SimpleString(&right))));
}

void print_String(struct String value)
{
    fprintf(stderr, "'%.*s'", (int)value.length, value.data);
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
    // Basic start and ends
    {
        struct String fox = CONST_STRING("the quick brown fox jumped over the lazy dog");
        struct SimpleString theStart = SIMPLE_STRING("the quick brown fox");
        struct SimpleString theEnd = SIMPLE_STRING("the lazy dog");
        _test(errors, fox.startsWith(&fox, &theStart), true);
        _test(errors, fox.endsWith(&fox, &theEnd), true);
        _test(errors, fox.startsWith(&fox, &theEnd), false);
        _test(errors, fox.endsWith(&fox, &theStart), false);
        fox.dtor(&fox);
    }
    if(errors)
        fprintf(stderr, "Errors: %d\n", errors);
    return errors;
}
