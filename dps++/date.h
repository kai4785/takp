#ifndef DATE_H
#define DATE_H

#include "utility.h"

#include <string>
#include <string_view>
#include <cstdint>

int64_t parseDate(const std::string_view datestring);
bool unparseDate(int64_t now, StringBuf& datestring);

#endif // DATE_H
