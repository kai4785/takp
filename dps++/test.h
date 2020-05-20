#ifndef TEST_H
#define TEST_H

#include <iostream>

#define _test(_errors, _expr, _iseq) { \
    int newErrors = !(_iseq == (_expr)); \
    if(newErrors) \
    { \
        std::cerr << __FILE__ << ":" << __LINE__ << " Failed expression" << std::endl; \
        std::cerr << "  " << #_expr << std::endl; \
    } \
    errors += newErrors; \
}

// TODO: Use stream operators instead of _print
#define _test_eq(_errors, _cmp, _print, _lhs, _rhs, _iseq, _c) { \
    int newErrors = _cmp(_lhs, _rhs, _iseq); \
    if(newErrors) \
    { \
        std::cerr << __FILE__ ":" << __LINE__ << " Failed equivalency check" << std::endl; \
        std::cerr << "  (" << #_lhs << " := "; \
        _print(_lhs); \
        std::cerr << ") " << _c; \
        std::cerr << "= (" << #_rhs << ") := "; \
        _print(_rhs); \
        std::cerr << ")" << std::endl; \
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
    std::cerr << value;
}

int cmp_Unsigned(uint64_t left, uint64_t right, bool equals)
{
    return (equals != (left == right));
}

void print_Unsigned(uint64_t value)
{
    std::cerr << value;
}

// NOTE: define 'errors' locally
#define test_int_eq(_x, _y) _test_eq(errors, cmp_Signed, print_Signed, _x, _y, true, '=')
#define test_int_neq(_x, _y) _test_eq(errors, cmp_Signed, print_Signed, _x, _y, true, '=')

#define test_uint_eq(_x, _y) _test_eq(errors, cmp_Unsigned, print_Signed, _x, _y, true, '=')
#define test_uint_neq(_x, _y) _test_eq(errors, cmp_Unsigned, print_Signed, _x, _y, true, '=')

#endif // TEST_H
