#include "tail.h"
#include "config.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

off_t fdSize(int fd)
{
    struct stat buf;
    int err = fstat(fd, &buf);
    if (err < 0)
        return -1;
    return buf.st_size;
}

void tail(const char* filename, tailfn callback)
{
    struct Config* config = configInstance();
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        fprintf(stderr, "Error opening file: [%d] %s\n", errno, filename);
        return;
    }
    off_t pos = config->history ? 0 : fdSize(fd);
    if(pos == (off_t)-1)
    {
        fprintf(stderr, "Error checking file size: [%d] %s\n", errno, filename);
        return;
    }
    printf("tail %s\n", filename);
    char line[64 * 1024] = {0};
    while(1)
    {
        ssize_t fileSize = fdSize(fd);
        ssize_t newPos = lseek(fd, pos, SEEK_SET);
        if(newPos > fileSize)
        {
            pos = fileSize;
            fprintf(stderr, "File truncation detected. New size: %zd\n", pos);
        }
        ssize_t readSize = read(fd, &line, sizeof(line));
        if(readSize == 0)
        {
            if(!config->follow)
                break;
            sleep(1);
        }
        size_t last = 0;
        for (size_t here = last; here < readSize; here++)
        {
            if (line[here] == '\r')
            {
                line[here] = '\0';
            }
            else if (line[here] == '\n')
            {
                line[here] = '\0';
                struct SimpleString string = {
                    .data = &line[last],
                    .length = here - last - 1,
                };
                callback(string);
                last = here + 1;
            }
        }
        pos += last;
    }
}
