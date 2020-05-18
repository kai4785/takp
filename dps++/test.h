#ifndef TEST_H
#define TEST_H

#include <stdio.h>

#define _test(_errors, _expr, _iseq) { \
    int newErrors = !(_iseq == (_expr)); \
    if(newErrors) \
    { \
        fprintf(stderr, "%s:%d Failed expression\n", __FILE__, __LINE__); \
        fprintf(stderr, "  %s\n", #_expr); \
    } \
    errors += newErrors; \
}

#define _test_eq(_errors, _cmp, _print, _lhs, _rhs, _iseq) { \
    int newErrors = _cmp(_lhs, _rhs, _iseq); \
    if(newErrors) \
    { \
        fprintf(stderr, "%s:%d Failed equivalency check\n", __FILE__, __LINE__); \
        fprintf(stderr, "  (%s := ", #_lhs); \
        _print(_lhs); \
        fprintf(stderr, ") %c= (%s := ", _iseq ? '!' : '=', #_rhs);\
        _print(_rhs); \
        fprintf(stderr, ")\n"); \
    } \
    _errors += newErrors; \
}

// Helpers
int cmp_Signed(int64_t left, int64_t right, bool equals)
{
    return (equals != (left == right));
}

void print_Signed(int64_t value)
{
    fprintf(stderr, "(%"PRId64")", value);
}

int cmp_Unsigned(uint64_t left, uint64_t right, bool equals)
{
    return (equals != (left == right));
}

void print_Unsigned(uint64_t value)
{
    fprintf(stderr, "(%"PRIu64")", value);
}

// NOTE: define 'errors' locally
#define test_int_eq(_x, _y) _test_eq(errors, cmp_Signed, print_Signed, _x, _y, true)
#define test_int_neq(_x, _y) _test_eq(errors, cmp_Signed, print_Signed, _x, _y, true)

#define test_uint_eq(_x, _y) _test_eq(errors, cmp_Unsigned, print_Signed, _x, _y, true)
#define test_uint_neq(_x, _y) _test_eq(errors, cmp_Unsigned, print_Signed, _x, _y, true)

#endif // TEST_H
