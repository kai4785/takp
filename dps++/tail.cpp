#include "tail.h"
#include "config.h"
#include "system.h"
#include "utility.h"

#include <string_view>
#include <cerrno>
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

off_t fdSize(int fd)
{
    struct stat buf;
    int err = fstat(fd, &buf);
    if (err < 0)
        return -1;
    return buf.st_size;
}

void tail(const string& filename, tailfn callback)
{
    bool keepgoing = true;
    struct Config* config = configInstance();

    int fd = ::open(filename.c_str(), O_RDONLY | O_BINARY);
    if (fd < 0)
    {
        cerr << "Error opening file: [" << errno << "] " << filename << endl;
        return;
    }

    off_t pos = config->history ? 0 : fdSize(fd);
    if(pos == (off_t)-1)
    {
        cerr << "Error checking file sile: [" << errno << "] " << filename << endl;
        return;
    }

    char line[64 * 1024] = {0};
    while(keepgoing)
    {
        off_t fileSize = fdSize(fd);
        off_t newPos = lseek(fd, pos, SEEK_SET);
        if(newPos > fileSize)
        {
            pos = fileSize;
            cerr << "File truncation detected. New size: " << pos << endl;
        }
        off_t readSize = read(fd, &line, sizeof(line));
        if(readSize == 0)
        {
            if(!config->follow)
                break;
            sleep(1);
        }
        size_t last = 0;
        for (size_t here = 0; here < readSize; here++)
        {
            if(line[here] == '\r' || line[here] == '\n')
            {
                if(config->verbosity > 10)
                {
                    cout << "Found an end of line character '" << std::hex << line[here] << "' here:" << here << ", last:" << last << endl;
                }
                if(last < here)
                {
                    string_view str(&line[last], here - last);
                    keepgoing = callback(str);
                }
                last = here + 1;
            }
        }
        pos += last;
    }
    close(fd);
}
