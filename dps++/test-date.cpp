#include "date.h"
#include "test.h"
#include "utility.h"

using namespace std;

#define dateEquals(_x, _y) _dateEquals(_x, _y, __FILE__, __LINE__)

// TODO: Why can't this be const?
int _dateEquals(const string_view& dateString, int64_t expected, const char* file, int line)
{
    int errors = 0;
    int64_t got = parseDate(dateString);
    if(got != expected)
    {
        cerr << "Failed to parse date: " << file << ":" << line << endl;
        cerr << "    date:  [" << dateString.size() << "]" << dateString << endl;
        cerr << "    value: " << got << " != " << endl;
        errors++;
        return errors;
    }
    StringBuf reparsed(20);
    if(!unparseDate(got, reparsed))
    {
        cerr << "Failed to reparse date: " << file << ":" << line << endl;
        cerr << "    date:  [" << got << "]" << dateString << endl;
    }
    else
    {
        if(dateString != reparsed)
        {
            cerr << "Failed to reparse date: " << file << ":" << line << endl;
            cerr << "    value:  [" << reparsed.size() << reparsed.string_view() << " != " << dateString << endl;
            errors++;
        }
    }
    return errors;
}

int testDates()
{
    // Used https://www.timeanddate.com/date/timeduration.html to come up with
    // the seconds between "Mar 16 00:00:00 1999" and a given date
    int errors = 0;

    errors += dateEquals("Mar 16 00:00:00 1999",           0LL);
    errors += dateEquals("Mar 16 00:00:01 1999",           1LL);
    errors += dateEquals("Mar 16 00:01:00 1999",          60LL);
    errors += dateEquals("Mar 16 01:00:00 1999",        3600LL);
    errors += dateEquals("Mar 17 00:00:00 1999",       86400LL);
    errors += dateEquals("Apr 16 00:00:00 1999",     2678400LL);
    // Leap year!
    errors += dateEquals("Jan 01 00:00:00 2000",    25142400LL);
    errors += dateEquals("Feb 01 00:00:00 2000",    27820800LL);
    errors += dateEquals("Feb 28 00:00:00 2000",    30153600LL);
    errors += dateEquals("Feb 29 00:00:00 2000",    30240000LL);
    errors += dateEquals("Mar 01 00:00:00 2000",    30326400LL);
    errors += dateEquals("Mar 02 00:00:00 2000",    30412800LL);
    errors += dateEquals("Mar 16 00:00:00 2000",    31622400LL);
    errors += dateEquals("Nov 30 00:00:00 2000",    54000000LL);
    errors += dateEquals("Nov 30 23:59:59 2000",    54086399LL);
    errors += dateEquals("Dec 01 00:00:00 2000",    54086400LL);
    errors += dateEquals("Dec 30 23:59:59 2000",    56678399LL);
    errors += dateEquals("Dec 31 00:00:00 2000",    56678400LL);
    errors += dateEquals("Dec 31 23:59:59 2000",    56764799LL);

    errors += dateEquals("Jan 01 00:00:00 2001",    56764800LL);
    errors += dateEquals("Feb 01 00:00:00 2001",    59443200LL);
    errors += dateEquals("Feb 28 00:00:00 2001",    61776000LL);
    errors += dateEquals("Mar 01 00:00:00 2001",    61862400LL);
    errors += dateEquals("Mar 02 00:00:00 2001",    61948800LL);
    errors += dateEquals("Mar 16 00:00:00 2001",    63158400LL);
    errors += dateEquals("Nov 30 00:00:00 2001",    85536000LL);
    errors += dateEquals("Nov 30 23:59:59 2001",    85622399LL);
    errors += dateEquals("Dec 01 00:00:00 2001",    85622400LL);
    errors += dateEquals("Dec 30 23:59:59 2001",    88214399LL);
    errors += dateEquals("Dec 31 00:00:00 2001",    88214400LL);
    errors += dateEquals("Dec 31 23:59:59 2001",    88300799LL);

    errors += dateEquals("Jan 01 00:00:00 2002",    88300800LL);
    errors += dateEquals("Feb 01 00:00:00 2002",    90979200LL);
    errors += dateEquals("Feb 28 00:00:00 2002",    93312000LL);
    errors += dateEquals("Mar 01 00:00:00 2002",    93398400LL);
    errors += dateEquals("Mar 02 00:00:00 2002",    93484800LL);
    errors += dateEquals("Mar 16 00:00:00 2002",    94694400LL);
    errors += dateEquals("Nov 30 00:00:00 2002",    117072000LL);
    errors += dateEquals("Nov 30 23:59:59 2002",    117158399LL);
    errors += dateEquals("Dec 01 00:00:00 2002",    117158400LL);
    errors += dateEquals("Dec 30 23:59:59 2002",    119750399LL);
    errors += dateEquals("Dec 31 00:00:00 2002",    119750400LL);
    errors += dateEquals("Dec 31 23:59:59 2002",    119836799LL);

    errors += dateEquals("Jan 01 00:00:00 2003",    119836800LL);
    errors += dateEquals("Feb 01 00:00:00 2003",    122515200LL);
    errors += dateEquals("Feb 28 00:00:00 2003",    124848000LL);
    errors += dateEquals("Mar 01 00:00:00 2003",    124934400LL);
    errors += dateEquals("Mar 02 00:00:00 2003",    125020800LL);
    errors += dateEquals("Mar 16 00:00:00 2003",    126230400LL);
    errors += dateEquals("Nov 30 00:00:00 2003",    148608000LL);
    errors += dateEquals("Nov 30 23:59:59 2003",    148694399LL);
    errors += dateEquals("Dec 01 00:00:00 2003",    148694400LL);
    errors += dateEquals("Dec 30 23:59:59 2003",    151286399LL);
    errors += dateEquals("Dec 31 00:00:00 2003",    151286400LL);
    errors += dateEquals("Dec 31 23:59:59 2003",    151372799LL);
    // Leap year!
    errors += dateEquals("Jan 01 00:00:00 2004",    151372800LL);
    errors += dateEquals("Feb 01 00:00:00 2004",    154051200LL);
    errors += dateEquals("Feb 28 00:00:00 2004",    156384000LL);
    errors += dateEquals("Feb 29 00:00:00 2004",    156470400LL);
    errors += dateEquals("Mar 01 00:00:00 2004",    156556800LL);
    errors += dateEquals("Mar 02 00:00:00 2004",    156643200LL);
    errors += dateEquals("Mar 16 00:00:00 2004",    157852800LL);
    errors += dateEquals("Nov 30 00:00:00 2004",    180230400LL);
    errors += dateEquals("Nov 30 23:59:59 2004",    180316799LL);
    errors += dateEquals("Dec 01 00:00:00 2004",    180316800LL);
    errors += dateEquals("Dec 30 23:59:59 2004",    182908799LL);
    errors += dateEquals("Dec 31 00:00:00 2004",    182908800LL);
    errors += dateEquals("Dec 31 23:59:59 2004",    182995199LL);

    errors += dateEquals("Jan 01 00:00:00 2005",    182995200LL);

    // Contemporary dates
    errors += dateEquals("Mar 06 22:09:37 2019",    630367777LL);
    errors += dateEquals("Mar 06 22:10:46 2019",    630367846LL);

    return errors;
}

int main()
{
    int errors = 0;
    errors += testDates();
    if(errors)
        fprintf(stderr, "Failed %d tests\n", errors);
    return errors;
}
