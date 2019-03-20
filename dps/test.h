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

#endif // TEST_H
