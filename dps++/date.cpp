#include "date.h"

#include "utility.h"
#include <cerrno>

using namespace std;

#define MINUTE (60LL)
#define HOUR (60*MINUTE)
#define DAY (24*HOUR)

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

struct DateValue
{
    int64_t second;
    int64_t minute;
    int64_t hour;
    int64_t day;
    int64_t month;
    int64_t year;
};
// Just parse one date string
// Wed Mar 06 22:09:37 2019
// 0   4   8  11:14:17 20
// Returns "seconds since 'Tue Mar 16 00:00:00 1999'", Everquest's Release date
// EQDate is calculated by running the code below, with EQDate set to 0.
const int64_t EQDate = 6393600;
int64_t parseDate(const string_view datestring)
{
    if(datestring.size() != 20)
    {
        return -1;
        errno = EINVAL;
    }
    int64_t seconds = 0;
    struct DateValue value = {0};
    string_view conv;
    // Second
    conv = datestring.substr(13, 2);
    value.second = toInt(conv);
    // Minute
    conv = datestring.substr(10, 2);
    value.minute = toInt(conv);
    // Hour
    conv = datestring.substr(7, 2);
    value.hour = toInt(conv);
    // Day
    // 1 based, so subtract a day to make it 0 based
    conv = datestring.substr(4, 2);
    value.day = toInt(conv);
    // Month
    // If this proves to be slow, parse each char one at a time.
    conv = datestring.substr(0, 3);
    if(conv == "Jan"sv)
        value.month = 1;
    else if(conv == "Feb"sv)
        value.month = 2;
    else if(conv == "Mar"sv)
        value.month = 3;
    else if(conv == "Apr"sv)
        value.month = 4;
    else if(conv == "May"sv)
        value.month = 5;
    else if(conv == "Jun"sv)
        value.month = 6;
    else if(conv == "Jul"sv)
        value.month = 7;
    else if(conv == "Aug"sv)
        value.month = 8;
    else if(conv == "Sep"sv)
        value.month = 9;
    else if(conv == "Oct"sv)
        value.month = 10;
    else if(conv == "Nov"sv)
        value.month = 11;
    else if(conv == "Dec"sv)
        value.month = 12;
    // Year
    conv = datestring.substr(16, 4);
    value.year = toInt(conv);
    
    // Convert month and year to days
    uint64_t monthDays = 0;
    // Count up days in the year before this month started
    switch(value.month - 1)
    {
        case 12: monthDays += 31;
        case 11: monthDays += 30;
        case 10: monthDays += 31;
        case 9: monthDays += 30;
        case 8: monthDays += 31;
        case 7: monthDays += 31;
        case 6: monthDays += 30;
        case 5: monthDays += 31;
        case 4: monthDays += 30;
        case 3: monthDays += 31;
        case 2: monthDays += isLeapYear(value.year) ? 29 : 28;
        case 1: monthDays += 31;
        case 0: break;
    };
    // Years since 1999
    int64_t yearDays = 365 * (value.year - 1999);
    // Leap years since 1999
    yearDays += leapYearsSince1999(value.year);
    // Add it all up
    seconds = value.second
        + value.minute * MINUTE
        + value.hour * HOUR
        + (value.day - 1) * DAY // Days are 1 based
        + monthDays * DAY
        + yearDays * DAY;

    // Now base it from EQDate
    seconds -= EQDate;
    return seconds;
}

bool unparseDate(int64_t now, StringBuf& datestring)
{
    // Space between month and day
    datestring[3] = ' ';
    // Space between day and hour
    datestring[6] = ' ';
    // Colon between hour and second
    datestring[9] = ':';
    // Colon between minute and second
    datestring[12] = ':';
    // Space between time and year
    datestring[15] = ' ';
    string_view conv;
    int64_t seconds = now + EQDate;

    // Year 4 characters
    datestring[16] = '?';
    datestring[17] = '?';
    datestring[18] = '?';
    datestring[19] = '?';
    // Month 3 characters
    datestring[0] = '?';
    datestring[1] = '?';
    datestring[2] = '?';

    int64_t day = seconds / DAY;
    seconds -= day * DAY;
    int64_t year = 1999; // start at 1999
    while(day >= 365 + isLeapYear(year))
    {
        day -= 365 + isLeapYear(year);
        year += 1;
    }
    // Year 4 characters
    datestring.fromInt(16, 4, year);

    int64_t leapday = isLeapYear(year);
    if(day < 31)
    {
        // Jan
        datestring.assign("Jan"sv, 0, 3);
    }
    else if(day < 59 + leapday)
    {
        // Feb
        day -= 31;
        datestring.assign("Feb"sv, 0, 3);
    }
    else if(day < 90 + leapday)
    {
        // Mar
        datestring.assign("Mar"sv, 0, 3);
        day -= 59 + leapday;
    }
    else if(day < 120 + leapday)
    {
        // Apr
        datestring.assign("Apr"sv, 0, 3);
        day -= 90 + leapday;
    }
    else if(day < 151 + leapday)
    {
        // May
        datestring.assign("May"sv, 0, 3);
        day -= 120 + leapday;
    }
    else if(day < 181 + leapday)
    {
        // Jun
        datestring.assign("Jun"sv, 0, 3);
        day -= 151 + leapday;
    }
    else if(day < 212 + leapday)
    {
        // Jul
        datestring.assign("Jul"sv, 0, 3);
        day -= 181 + leapday;
    }
    else if(day < 243 + leapday)
    {
        // Aug
        datestring.assign("Aug"sv, 0, 3);
        day -= 212 + leapday;
    }
    else if(day < 273 + leapday)
    {
        // Sep
        datestring.assign("Sep"sv, 0, 3);
        day -= 243 + leapday;
    }
    else if(day < 304 + leapday)
    {
        // Oct
        datestring.assign("Oct"sv, 0, 3);
        day -= 273 + leapday;
    }
    else if(day < 334 + leapday)
    {
        // Nov
        datestring.assign("Nov"sv, 0, 3);
        day -= 304 + leapday;
    }
    else if(day < 365 + leapday)
    {
        // Dec
        datestring.assign("Dec"sv, 0, 3);
        day -= 334 + leapday;
    }
    // Day 2 characters
    datestring.fromInt(4, 2, day + 1);
    // Hour 2 characters
    int64_t hour = seconds / HOUR;
    datestring.fromInt(7, 2, hour);
    seconds -= hour * HOUR;
    // Minute 2 characters
    int64_t minute = seconds / MINUTE;
    datestring.fromInt(10, 2, minute);
    seconds -= minute * MINUTE;
    // Seconds 2 characters
    datestring.fromInt(13, 2, seconds);
    return true;
}
