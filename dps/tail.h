#ifndef TAIL_H
#define TAIL_H

#include "utility.h"

#include <stddef.h>

typedef void (*tailfn)(struct SimpleString line);

void tail(const char* filename, tailfn callback);

#endif // TAIL_H
