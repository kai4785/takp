#include "config.h"
#include "test.h"
#include "tail.h"
#include "string.h"
#include "utility.h"

#include "system.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

int cmp_String(struct SimpleString _left, struct SimpleString right, bool equals)
{
    struct String left;
    String_ctorHold(&left, _left.data, _left.length);
    return (equals != (left.op_equal(&left, &right)));
}

void print_String(struct SimpleString value)
{
    fprintf(stderr, "(%zu)'%.*s'", value.length, (int)value.length, value.data);
}

#define test_eq(_x, _y) _test_eq(errors, cmp_String, print_String, _x, _y, true)
#define test_neq(_x, _y) _test_eq(errors, cmp_String, print_String, _x, _y, false)

#define test(_expr) _test(errors, _expr, true);
#define test_false(_expr) _test(errors, _expr, false);

const char* testFileName = "test-tail.txt";

struct SimpleString lines[] = {
    { .data = "line0", .length = 5},
    { .data = "line1", .length = 5},
    { .data = "line2", .length = 5},
    { .data = "line3", .length = 5},
    { .data = "line4", .length = 5},
    { .data = "line5", .length = 5},
};

int errors = 0;
size_t lineno = 0;
void tellme(struct SimpleString _line)
{
    test_eq(_line, lines[lineno]);
    lineno++;
}

void thething(struct SimpleString endofline)
{
    int fd = creat(testFileName, S_IREAD|S_IWRITE);
    if(fd < 0)
    {
        fprintf(stderr, "Failed to open file: [%d]%s\n", errno, testFileName);
        ++errors;
        return;
    }
    for(size_t i = 0; i < 5; i++)
    {
        off_t bytes = 0;
        bytes = write(fd, lines[i].data, lines[i].length);
        if(bytes != lines[i].length)
        {
            fprintf(stderr, "Failed to write to file: [%d]%s\n", errno, testFileName);
            ++errors;
            return;
        }
        bytes = write(fd, endofline.data, endofline.length);
        if(bytes != endofline.length)
        {
            fprintf(stderr, "Failed to write newlines to file: [%d]%s\n", errno, testFileName);
            ++errors;
            return;
        }
    }
    close(fd);
    tail(testFileName, tellme);
    unlink(testFileName);
}

int main()
{
    struct Config* config = configInstance();
    config->history = true;
    struct SimpleString cr = SIMPLE_STRING("\r");
    struct SimpleString lf = SIMPLE_STRING("\n");
    struct SimpleString crlf = SIMPLE_STRING("\r\n");
    printf("cr\n");
    lineno = 0;
    thething(cr);
    printf("lf\n");
    lineno = 0;
    thething(lf);
    printf("crlf\n");
    lineno = 0;
    thething(crlf);
    return errors;
}
