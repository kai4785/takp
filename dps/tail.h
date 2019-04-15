#ifndef TAIL_H
#define TAIL_H

#include "utility.h"

#include <stddef.h>

// Return 'false' if you want the parsing to stop
typedef bool (*tailfn)(struct SimpleString line);

void tail(const char* filename, tailfn callback);

#endif // TAIL_H
