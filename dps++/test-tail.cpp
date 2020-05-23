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

#include <iostream>
#include <string_view>

using namespace std;

int cmp_String(const string_view& left, const string_view& right, bool equals)
{
    return (equals != (left == right));
}

void print_String(const string_view& value)
{
    cout << value;
}

#define test_eq(_x, _y) _test_eq(errors, cmp_String, print_String, _x, _y, true, '=')
#define test_neq(_x, _y) _test_eq(errors, cmp_String, print_String, _x, _y, false, '!')

#define test(_expr) _test(errors, _expr, true);
#define test_false(_expr) _test(errors, _expr, false);

const char* testFileName = "test-tail.txt";

string_view lines[] = {
    {"line0"sv},
    {"line1"sv},
    {"line2"sv},
    {"line3"sv},
    {"line4"sv},
    {"line5"sv},
    {"line6"sv},
    {"line7"sv},
    {"line8"sv},
    {"line9"sv},
    {"line10"sv},
    {"line11"sv},
    {"line12"sv},
    {"line13"sv},
    {"line14"sv},
    {"line15"sv},
    {"line16"sv},
    {"line17"sv},
    {"line18"sv},
    {"line19"sv},
    {"line20"sv},
};

int errors = 0;
size_t lineno = 0;
bool tellme(const string_view& line)
{
    if(Config::instance().verbosity > 5)
    {
        cout << "[" << lineno << "](" << line.size() << ")" << " " << line << endl;
    }
    test_eq(line, lines[lineno]);
    lineno++;
    return true;
}

void thething(const string_view& endofline)
{
    int fd = creat(testFileName, S_IREAD|S_IWRITE);
    if(fd < 0)
    {
        cout << "Failed to open file: [" << errno << "]" << testFileName;
        ++errors;
        return;
    }
    for(size_t i = 0; i < sizeof(lines) / sizeof(*lines); i++)
    {
        off_t bytes = 0;
        bytes = write(fd, lines[i].data(), lines[i].size());
        if(bytes != lines[i].size())
        {
            cout << "Failed to write to file: [" << errno << "]" << testFileName;
            ++errors;
            return;
        }
        bytes = write(fd, endofline.data(), endofline.size());
        if(bytes != endofline.size())
        {
            cout << "Failed to write new lines to file: [" << errno << "]" << testFileName;
            ++errors;
            return;
        }
    }
    close(fd);
    tail(testFileName, tellme);
    unlink(testFileName);
}

int doit()
{
    auto cr = "\r"sv;
    auto lf = "\n"sv;
    auto crlf = "\r\n"sv;
    cout << "cr\n";
    lineno = 0;
    thething(cr);
    cout << "lf\n";
    lineno = 0;
    thething(lf);
    cout << "crlf\n";
    lineno = 0;
    thething(crlf);
    return errors;
}

int main()
{
    int errors = 0;

    auto& config = Config::instance();
    config.history = true;
    config.verbosity = 10;
    cout << "IO_LOOP" << endl;
    config.io = Config::IO_LOOP;
    errors += doit();

    config.io = Config::IO_ASIO;
    cout << "IO_ASIO" << endl;
    errors += doit();
}
