#include "utility.h"

using namespace std;

bool fromInt(char* data, size_t length, int64_t value)
{
    char* here = data + length - 1;
    int64_t base = 10;
    for(size_t i = 0; i < length; i++)
    {
        *(here--) = (value % base) + '0';
        value /= 10;
    }
    return(value == 0);
}

int64_t toInt(const string_view& str)
{
    int64_t value = 0;
    int64_t base = 1;
    const char* here = str.data() + str.size() - 1;
    for(size_t i = 0; i < str.size(); i++)
    {
        value += (*(here--) - '0') * base;
        base *= 10;
    }
    return value;
}

bool endsWith(const string_view& str, const string_view& end)
{
    if(end.size() > str.size())
    {
        return false;
    }
    return (str.substr(str.size() - end.size()) == end);
}

bool startsWith(const string_view& str, const string_view& start)
{
    if(start.size() > str.size())
    {
        return false;
    }
    return (str.substr(0, start.size()) == start);
}