#ifndef TAIL_H
#define TAIL_H

#include <stddef.h>

typedef void (*tailfn)(const char* line, size_t length);

void tail(const char* filename, tailfn callback);

#endif // TAIL_H
