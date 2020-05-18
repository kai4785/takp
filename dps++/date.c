#include "date.h"

#include <string.h>
#include <stdbool.h>
#include <errno.h>

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
int64_t parseDate(const struct SimpleString datestring)
{
    if(datestring.length != 20)
    {
        return -1;
        errno = EINVAL;
    }
    int64_t seconds = 0;
    struct DateValue value = {0};
    struct String conv = {0};
    String_ctor(&conv);
    // Second
    conv.hold(&conv, datestring.data + 13, 2);
    value.second = conv.toInt(&conv);
    // Minute
    conv.hold(&conv, datestring.data + 10, 2);
    value.minute = conv.toInt(&conv);
    // Hour
    conv.hold(&conv, datestring.data + 7, 2);
    value.hour = conv.toInt(&conv);
    // Day
    // 1 based, so subtract a day to make it 0 based
    conv.hold(&conv, datestring.data + 4, 2);
    value.day = conv.toInt(&conv);
    // Month
    // If this proves to be slow, parse each char one at a time.
    conv.hold(&conv, datestring.data, 3);
    if(conv.cmp(&conv, "Jan", 3))
        value.month = 1;
    else if(conv.cmp(&conv, "Feb", 3))
        value.month = 2;
    else if(conv.cmp(&conv, "Mar", 3))
        value.month = 3;
    else if(conv.cmp(&conv, "Apr", 3))
        value.month = 4;
    else if(conv.cmp(&conv, "May", 3))
        value.month = 5;
    else if(conv.cmp(&conv, "Jun", 3))
        value.month = 6;
    else if(conv.cmp(&conv, "Jul", 3))
        value.month = 7;
    else if(conv.cmp(&conv, "Aug", 3))
        value.month = 8;
    else if(conv.cmp(&conv, "Sep", 3))
        value.month = 9;
    else if(conv.cmp(&conv, "Oct", 3))
        value.month = 10;
    else if(conv.cmp(&conv, "Nov", 3))
        value.month = 11;
    else if(conv.cmp(&conv, "Dec", 3))
        value.month = 12;
    // Year
    conv.hold(&conv, datestring.data + 16, 4);
    value.year = conv.toInt(&conv);
    
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

bool unparseDate(int64_t now, struct SimpleString* datestring)
{
    // Space between month and day
    datestring->data[3] = ' ';
    // Space between day and hour
    datestring->data[6] = ' ';
    // Colon between hour and second
    datestring->data[9] = ':';
    // Colon between minute and second
    datestring->data[12] = ':';
    // Space between time and year
    datestring->data[15] = ' ';
    struct String conv = {0};
    String_ctor(&conv);
    int64_t seconds = now + EQDate;

    // Year 4 characters
    datestring->data[16] = '?';
    datestring->data[17] = '?';
    datestring->data[18] = '?';
    datestring->data[19] = '?';
    // Month 3 characters
    datestring->data[0] = '?';
    datestring->data[1] = '?';
    datestring->data[2] = '?';

    int64_t day = seconds / DAY;
    seconds -= day * DAY;
    int64_t year = 1999; // start at 1999
    while(day >= 365 + isLeapYear(year))
    {
        day -= 365 + isLeapYear(year);
        year += 1;
    }
    // Year 4 characters
    conv.hold(&conv, datestring->data + 16, 4);
    conv.fromInt(&conv, year); // Days are 1 based

    int64_t leapday = isLeapYear(year);
    conv.hold(&conv, datestring->data, 3);
    if(day < 31)
    {
        // Jan
        conv.cpy(&conv, "Jan", 3);
    }
    else if(day < 59 + leapday)
    {
        // Feb
        day -= 31;
        conv.cpy(&conv, "Feb", 3);
    }
    else if(day < 90 + leapday)
    {
        // Mar
        conv.cpy(&conv, "Mar", 3);
        day -= 59 + leapday;
    }
    else if(day < 120 + leapday)
    {
        // Apr
        conv.cpy(&conv, "Apr", 3);
        day -= 90 + leapday;
    }
    else if(day < 151 + leapday)
    {
        // May
        conv.cpy(&conv, "May", 3);
        day -= 120 + leapday;
    }
    else if(day < 181 + leapday)
    {
        // Jun
        conv.cpy(&conv, "Jun", 3);
        day -= 151 + leapday;
    }
    else if(day < 212 + leapday)
    {
        // Jul
        conv.cpy(&conv, "Jul", 3);
        day -= 181 + leapday;
    }
    else if(day < 243 + leapday)
    {
        // Aug
        conv.cpy(&conv, "Aug", 3);
        day -= 212 + leapday;
    }
    else if(day < 273 + leapday)
    {
        // Sep
        conv.cpy(&conv, "Sep", 3);
        day -= 243 + leapday;
    }
    else if(day < 304 + leapday)
    {
        // Oct
        conv.cpy(&conv, "Oct", 3);
        day -= 273 + leapday;
    }
    else if(day < 334 + leapday)
    {
        // Nov
        conv.cpy(&conv, "Nov", 3);
        day -= 304 + leapday;
    }
    else if(day < 365 + leapday)
    {
        // Dec
        conv.cpy(&conv, "Dec", 3);
        day -= 334 + leapday;
    }
    // Day 2 characters
    conv.hold(&conv, datestring->data + 4, 2);
    conv.fromInt(&conv, day + 1); // Days are 1 based
    // Hour 2 characters
    int64_t hour = seconds / HOUR;
    conv.hold(&conv, datestring->data + 7, 2);
    conv.fromInt(&conv, hour);
    seconds -= hour * HOUR;
    // Minute 2 characters
    int64_t minute = seconds / MINUTE;
    conv.hold(&conv, datestring->data + 10, 2);
    conv.fromInt(&conv, minute);
    seconds -= minute * MINUTE;
    // Seconds 2 characters
    conv.hold(&conv, datestring->data + 13, 2);
    conv.fromInt(&conv, seconds);
    return true;
}
