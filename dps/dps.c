#include "tail.h"

#include <stdio.h>

void tellme(const char* line)
{
    static ssize_t lineno = 0;
    printf("tellme[%zd]: %s\n", ++lineno, line);
}

int main(int argc, char **argv)
{
    tail(argv[1], tellme);
    return 0;
}
