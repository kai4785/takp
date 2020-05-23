#ifndef TAIL_H
#define TAIL_H

#include <string>
#include <functional>

// Return 'false' if you want the parsing to stop
typedef std::function<bool (const std::string_view&)> TailCallBack;

void tail(const std::string_view& filename, const TailCallBack& callback);

#endif // TAIL_H
