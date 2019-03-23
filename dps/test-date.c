#include "date.h"
#include "test.h"

#define dateEquals(_x, _y) _dateEquals(_x, _y, __FILE__, __LINE__)

// TODO: Why can't this be const?
int _dateEquals(/*const*/ char* date, int64_t expected, const char* file, int line)
{
    int errors = 0;
    const struct SimpleString dateString =
    {
        .data = date,
        .length = strlen(date)
    };
    int64_t got = parseDate(dateString);
    if(got != expected)
    {
        fprintf(stderr, "Failed to parse date: %s:%d\n", file, line);
        fprintf(stderr, "   date:  [%d]%.*s\n", (int)dateString.length, (int)dateString.length, dateString.data);
        fprintf(stderr, "   value: %"PRId64" != %"PRId64"\n", got, expected);
        errors++;
    }
    return errors;
}

int testDates()
{
    int errors = 0;

    errors += dateEquals("Tue Mar 16 00:00:00 1999",           0LL);
    errors += dateEquals("Tue Mar 16 00:00:01 1999",           1LL);
    errors += dateEquals("Tue Mar 16 00:01:00 1999",          60LL);
    errors += dateEquals("Tue Mar 16 01:00:00 1999",        3600LL);
    errors += dateEquals("Tue Apr 16 00:00:00 1999",     2678400LL);
    errors += dateEquals("Tue Mar 16 00:00:00 2000",    31622400LL); // Leap year!
    errors += dateEquals("Wed Mar 06 22:09:37 2019",   630367777LL);
    errors += dateEquals("Wed Mar 06 22:10:46 2019",   630367846LL);

    return errors;
}

int main()
{
    int errors = 0;
    errors += testDates();
    return errors;
}
