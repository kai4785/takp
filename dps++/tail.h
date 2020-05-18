#ifndef TAIL_H
#define TAIL_H

#include <string>

#include <stddef.h>

// Return 'false' if you want the parsing to stop
typedef bool (*tailfn)(const std::string_view& line);

void tail(const std::string& filename, tailfn callback);

#endif // TAIL_H
