#include "date.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

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

static inline bool isLeapYear(int64_t year)
{
    // If this proves to be slow, just create an array of leap-years since 1999.
    return (((year % 4 == 0) && (year % 100 !=0)) || (year % 400 == 0));
}

// Just parse one date string
// Wed Mar 06 22:09:37 2019
// 0   4   8  11:14:17 20
// Returns "seconds since 'Jan 01 00:00:00 0000'"
int64_t parseDate(const char* datestring)
{
    int64_t seconds = 0;
    // Month
    // If this proves to be slow, parse each char one at a time.
    if(strncmp("Jan", &datestring[4], 3) == 0)
        seconds += JAN;
    else if(strncmp("Feb", &datestring[4], 3) == 0)
        seconds += FEB;
    else if(strncmp("Mar", &datestring[4], 3) == 0)
        seconds += MAR;
    else if(strncmp("Apr", &datestring[4], 3) == 0)
        seconds += APR;
    else if(strncmp("May", &datestring[4], 3) == 0)
        seconds += MAY;
    else if(strncmp("Jun", &datestring[4], 3) == 0)
        seconds += JUN;
    else if(strncmp("Jul", &datestring[4], 3) == 0)
        seconds += JUL;
    else if(strncmp("Aug", &datestring[4], 3) == 0)
        seconds += AUG;
    else if(strncmp("Sep", &datestring[4], 3) == 0)
        seconds += SEP;
    else if(strncmp("Oct", &datestring[4], 3) == 0)
        seconds += OCT;
    else if(strncmp("Nov", &datestring[4], 3) == 0)
        seconds += NOV;
    else if(strncmp("Dec", &datestring[4], 3) == 0)
        seconds += DEC;
    // Day
    seconds += (datestring[8] - '0') * DAY * 10;
    seconds += (datestring[9] - '0') * DAY;
    // Hour
    seconds += (datestring[11] - '0') * HOUR * 10;
    seconds += (datestring[12] - '0') * HOUR;
    // Minute
    seconds += (datestring[14] - '0') * MINUTE * 10;
    seconds += (datestring[15] - '0') * MINUTE;
    // Second
    seconds += (datestring[17] - '0') * 10;
    seconds += (datestring[18] - '0');
    // Year
    int64_t year = 0;
    year += (datestring[20] - '0') * 1000;
    year += (datestring[21] - '0') * 100;
    year += (datestring[22] - '0') * 10;
    year += (datestring[23] - '0');
    // If it's after February, but it's a leap year, add a day.
    // Feb 29 will handle itself.
    if(seconds >= MAR && isLeapYear(year))
    {
        seconds += DAY;
    }

    seconds += year * YEAR;
    return seconds;
}
