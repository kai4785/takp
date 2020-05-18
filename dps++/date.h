#ifndef DATE_H
#define DATE_H

#include "utility.h"

#include <inttypes.h>

int64_t parseDate(const struct SimpleString datestring);
bool unparseDate(int64_t now, struct SimpleString* datestring);

#endif // DATE_H
