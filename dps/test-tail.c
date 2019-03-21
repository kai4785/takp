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
    SIMPLE_STRING("line0"),
    SIMPLE_STRING("line1"),
    SIMPLE_STRING("line2"),
    SIMPLE_STRING("line3"),
	SIMPLE_STRING("[Thu Feb 14 10:42:52 2019] Welcome to EverQuest!"),
	SIMPLE_STRING("[Thu Feb 14 10:42:52 2019] You have entered Velketor's Labyrinth."),
	SIMPLE_STRING("[Thu Feb 14 10:42:52 2019] MESSAGE OF THE DAY: Welcome to The Al'Kabor Project.  Be sure to report any bugs you find to takproject.net bug section of the forum."),
	SIMPLE_STRING("[Thu Feb 14 10:42:52 2019] Autojoining channels..."),
	SIMPLE_STRING("[Thu Feb 14 10:42:52 2019] Channels: 1=Alliance(94)"),
	SIMPLE_STRING("[Thu Feb 14 10:42:54 2019] Players on EverQuest:"),
	SIMPLE_STRING("[Thu Feb 14 10:42:54 2019] ---------------------------"),
	SIMPLE_STRING("[Thu Feb 14 10:42:54 2019] [51 Minstrel] Iarwain (Half Elf) <Black Sheep>"),
	SIMPLE_STRING("[Thu Feb 14 10:42:54 2019] [60 Hierophant] Peregrin (Halfling) <Black Sheep>"),
	SIMPLE_STRING("[Thu Feb 14 10:42:54 2019] [60 Warlord] Meriadoc (Halfling) <Black Sheep>"),
	SIMPLE_STRING("[Thu Feb 14 10:42:54 2019] There are 3 players in Velketor's Labyrinth."),
	SIMPLE_STRING("[Thu Feb 14 10:43:03 2019] You invite Iarwain to join your group."),
	SIMPLE_STRING("[Thu Feb 14 10:43:03 2019] You have formed the group."),
	SIMPLE_STRING("[Thu Feb 14 10:43:03 2019] Iarwain has joined the group."),
	SIMPLE_STRING("[Thu Feb 14 10:43:07 2019] Iarwain's image shimmers."),
	SIMPLE_STRING("[Thu Feb 14 10:43:09 2019] It will take you about 30 seconds to prepare your camp."),
	SIMPLE_STRING("[Thu Feb 14 10:43:13 2019] It will take about 25 more seconds to prepare your camp."),
	SIMPLE_STRING("[Thu Feb 14 10:43:18 2019] It will take about 20 more seconds to prepare your camp."),
	SIMPLE_STRING("[Thu Feb 14 10:43:23 2019] It will take about 15 more seconds to prepare your camp."),
	SIMPLE_STRING("[Thu Feb 14 10:43:28 2019] It will take about 10 more seconds to prepare your camp."),
	SIMPLE_STRING("[Thu Feb 14 10:43:33 2019] It will take about 5 more seconds to prepare your camp."),
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
        if(!bytes)
        {
            fprintf(stderr, "Failed to write to file: [%d]%s\n", errno, testFileName);
            ++errors;
            return;
        }
        bytes = write(fd, endofline.data, endofline.length);
        if(!bytes)
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
