#include "date.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

#define MINUTE (60LL)
#define HOUR (60*MINUTE)
#define DAY (24*HOUR)
// JAN starts on 0, and FEB is 31 days of January
#define JAN 0
#define FEB (JAN+(DAY*31))
#define MAR (FEB+(DAY*28)) // See isLeapYear()
#define APR (MAR+(DAY*31))
#define MAY (APR+(DAY*30))
#define JUN (MAY+(DAY*31))
#define JUL (JUN+(DAY*30))
#define AUG (JUL+(DAY*31))
#define SEP (AUG+(DAY*31))
#define OCT (SEP+(DAY*30))
#define NOV (OCT+(DAY*31))
#define DEC (NOV+(DAY*30))
#define YEAR (DEC+(DAY*31))

// Are we in a leap year?
static inline bool isLeapYear(int64_t year)
{
    return (year & 3) == 0 && ((year % 25) != 0 || (year & 15) == 0);
}

// How many leap years since 1999?
// Does not count current year, since it's dependent on the date
static inline int leapYearsSince1999(int64_t year)
{
    int count = 0;
    for(int64_t i = 2000; i < year; i+=4)
    {
        count += isLeapYear(i);
    }
    return count;
}

// Just parse one date string
// Wed Mar 06 22:09:37 2019
// 0   4   8  11:14:17 20
// Returns "seconds since 'Tue Mar 16 00:00:00 1999'", Everquest's Release date
// EQDate is calculated by running the code below, with EQDate set to 0.
const int64_t EQDate = 63046857600;
int64_t parseDate(const struct String datestring)
{
    if(datestring.length != 24)
    {
        return -1;
        errno = EINVAL;
    }
    int64_t seconds = 0;
    // Month
    // If this proves to be slow, parse each char one at a time.
    if(strncmp("Jan", &datestring.data[4], 3) == 0)
        seconds += JAN;
    else if(strncmp("Feb", &datestring.data[4], 3) == 0)
        seconds += FEB;
    else if(strncmp("Mar", &datestring.data[4], 3) == 0)
        seconds += MAR;
    else if(strncmp("Apr", &datestring.data[4], 3) == 0)
        seconds += APR;
    else if(strncmp("May", &datestring.data[4], 3) == 0)
        seconds += MAY;
    else if(strncmp("Jun", &datestring.data[4], 3) == 0)
        seconds += JUN;
    else if(strncmp("Jul", &datestring.data[4], 3) == 0)
        seconds += JUL;
    else if(strncmp("Aug", &datestring.data[4], 3) == 0)
        seconds += AUG;
    else if(strncmp("Sep", &datestring.data[4], 3) == 0)
        seconds += SEP;
    else if(strncmp("Oct", &datestring.data[4], 3) == 0)
        seconds += OCT;
    else if(strncmp("Nov", &datestring.data[4], 3) == 0)
        seconds += NOV;
    else if(strncmp("Dec", &datestring.data[4], 3) == 0)
        seconds += DEC;
    // Day
    seconds += (datestring.data[8] - '0') * DAY * 10;
    seconds += (datestring.data[9] - '0') * DAY;
    seconds -= DAY; // Days are 1 based, so subtract a day to make it 0 based
    // Hour
    seconds += (datestring.data[11] - '0') * HOUR * 10;
    seconds += (datestring.data[12] - '0') * HOUR;
    // Minute
    seconds += (datestring.data[14] - '0') * MINUTE * 10;
    seconds += (datestring.data[15] - '0') * MINUTE;
    // Second
    seconds += (datestring.data[17] - '0') * 10;
    seconds += (datestring.data[18] - '0');
    // Year
    int64_t year = 0;
    year += (datestring.data[20] - '0') * 1000;
    year += (datestring.data[21] - '0') * 100;
    year += (datestring.data[22] - '0') * 10;
    year += (datestring.data[23] - '0');
    // Leap years
    seconds += DAY * leapYearsSince1999(year);
    // If it's after February, but it's a leap year, add a day.
    // Feb 29 will handle itself.
    if(seconds >= MAR && isLeapYear(year))
    {
        seconds += DAY;
    }

    seconds += year * YEAR;
    seconds -= EQDate;
    return seconds;
}
