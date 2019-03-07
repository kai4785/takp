#include "tail.h"
#include "config.h"
#include "date.h"

#include <string.h>
#include <stdio.h>

 __attribute((constructor)) void configInstance(void)
{
    config.follow = false;
    config.history = true;
    config.me = NULL;
    config.logfile = NULL;
    config.since = NULL;
    config.keepalive = 10;
}

void tellme(const char* line, size_t length)
{
    // empty line
    if(length == 0)
        return;
    char datestring[25] = {0};
    const char* message = NULL;
    static ssize_t lineno = 0;
    int64_t dateseconds = 0;
    if(length < 27 || !(line[0] == '[' && line[25] == ']'))
    {
        fprintf(stderr, "[%zd] No date in message(%zu): (%02x %02x) [|%s|]\n", ++lineno, length, line[0], line[25], line);
        return;
    }
    strncpy(datestring, &line[1], 24);
    message = &line[27];
    dateseconds = parseDate(datestring);
    length -= 27;
    printf("[%zd]:[%ld] %s (%zu | %zu)\n", ++lineno, dateseconds, message, length, strnlen(message, 100));
    /* Messages are one of the following.
     * End of Battle
          LOADING, PLEASE WAIT...
     * Melee Damage
     *    Verbs "smash|smashes|hit|slash|claw|claws|crush|pierce|kick|bash|maul|gore|gores|slice|slices|slashes|crushes|hits|punch|punches|kicks|bashes|bites|pierces|mauls|backstab|backstabs"
     *    ^([A-Za-z `]+) ([A-Za-z `]+) ([A-Za-z `]+) for ([0-9]+) points of damage\.
     * Magic Damage
     *    ([A-Za-z `]+) was (hit by non-melee) for ([0-9]+) points of damage\.
     * Cripple Damage (duplicate of Melee)
     *    ^([A-Za-z `]+) lands a Crippling Blow\!\(([0-9]+)\)
     * Critical Damage (duplicate of Melee/Magic)
     *    ^([A-Za-z `]+) Scores a critical hit\!\(([0-9]+)\)
     * Heal Damage
     *    (You) have been (healed) for ([0-9]+) points of damage\.
     * Death
     *    ([A-Za-z `]+) have slain ([A-Za-z `]+)!
     *    ([A-Za-z `]+) (has|have) been slain by ([A-Za-z `]+)!
     *    ([A-Za-z `]+) died\.
     */
}

int main(int argc, char **argv)
{
    tail(argv[1], tellme);
    return 0;
}
