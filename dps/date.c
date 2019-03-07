#include "date.h"

#include <string.h>

// Just parse one date string
// Wed Mar 06 22:09:37 2019
// 0   4   8  11:14:17 20
int64_t parseDate(const char* datestring)
{
    uint64_t seconds = 0;
    // Second
    seconds += (datestring[18] - '0');
    seconds += (datestring[17] - '0') * 10;
    // Minute
    seconds += (datestring[15] - '0') * 60;
    seconds += (datestring[14] - '0') * 60 * 10;
    // Hour
    seconds += (datestring[12] - '0') * 60 * 60;
    seconds += (datestring[11] - '0') * 60 * 60 * 10;
    // Day
    seconds += (datestring[9] - '0') * 24 * 60 * 60;
    seconds += (datestring[8] - '0') * 24 * 60 * 60 * 10;
    // Month
    if(strncmp("Jan", &datestring[4], 3))
        seconds += 1 * 31 * 24 * 60 * 60;
    // Wrong on leap year. Oh well. Cross that bridge when we get there.
    if(strncmp("Feb", &datestring[4], 3))
        seconds += 2 * 28 * 24 * 60 * 60;
    if(strncmp("Mar", &datestring[4], 3))
        seconds += 3 * 31 * 24 * 60 * 60;
    if(strncmp("Apr", &datestring[4], 3))
        seconds += 4 * 30 * 24 * 60 * 60;
    if(strncmp("May", &datestring[4], 3))
        seconds += 5 * 31 * 24 * 60 * 60;
    if(strncmp("Jun", &datestring[4], 3))
        seconds += 6 * 30 * 24 * 60 * 60;
    if(strncmp("Jul", &datestring[4], 3))
        seconds += 7 * 31 * 24 * 60 * 60;
    if(strncmp("Aug", &datestring[4], 3))
        seconds += 8 * 31 * 24 * 60 * 60;
    if(strncmp("Sep", &datestring[4], 3))
        seconds += 9 * 30 * 24 * 60 * 60;
    if(strncmp("Oct", &datestring[4], 3))
        seconds += 10 * 31 * 24 * 60 * 60;
    if(strncmp("Nov", &datestring[4], 3))
        seconds += 11 * 30 * 24 * 60 * 60;
    if(strncmp("Dec", &datestring[4], 3))
        seconds += 12 * 31 * 24 * 60 * 60;
    // Year
    seconds += (datestring[23] - '0') * 12 * 31 * 24 * 60 * 60;
    seconds += (datestring[22] - '0') * 10 * 12 * 31 * 24 * 60 * 60;
    seconds += (datestring[21] - '0') * 100 * 12 * 31 * 24 * 60 * 60;
    seconds += (datestring[20] - '0') * 1000 * 12 * 31 * 24 * 60 * 60;
    return seconds;
}
