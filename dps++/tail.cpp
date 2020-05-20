#include "tail.h"
#include "config.h"
#include "system.h"
#include "utility.h"

#include <string_view>
#include <cerrno>
#include <iostream>
#include <functional>
#include <fcntl.h>
#include <utility>
#include <sys/stat.h>
#include <sys/types.h>

#include <asio.hpp>

using namespace std;
using namespace asio;

off_t fdSize(int fd)
{
    struct stat buf;
    int err = fstat(fd, &buf);
    if (err < 0)
        return -1;
    return buf.st_size;
}

typedef asio::buffers_iterator<asio::streambuf::const_buffers_type> bi;
std::pair<bi, bool>
match_newline(bi begin, bi end)
{
    bi here = begin;
    while(here != end)
    {
        char c = *here++;
        if(c == '\r' || c == '\n')
        {
            return std::make_pair(here, true);
        }
    }
    return std::make_pair(here, false);
}

class TailAsio
{
public:
    TailAsio(int fd, tailfn callback, off_t pos)
        :m_fd(fd)
        ,m_callback(callback)
        ,m_pos(pos)
        ,m_fileSize{fdSize(fd)}
        ,m_stream{m_ioservice, m_fd}
        ,m_config(configInstance())
    {
    }

    void handler(const std::error_code& error, const long unsigned int& bytesRead)
    {
        if(bytesRead > 1)
        {
            std::string_view line{asio::buffer_cast<const char*>(m_buf.data()), bytesRead - 1};
            m_callback(line);
        }
        m_buf.consume(bytesRead);

        m_pos += bytesRead;
        if(m_pos == m_fileSize && !m_config.follow)
        {
            cout << "Read the last bytes." << endl;
            m_ioservice.stop();
        }
        else
        {
            asio::async_read_until(m_stream, m_buf, match_newline, std::bind(&TailAsio::handler, this, std::placeholders::_1, std::placeholders::_2));
        }
    }

    void run()
    {
        asio::async_read_until(m_stream, m_buf, match_newline, std::bind(&TailAsio::handler, this, std::placeholders::_1, std::placeholders::_2));
        m_ioservice.run();
        cout << "Done running" << endl;
    }

private:
    int m_fd;
    tailfn m_callback;
    off_t m_pos;
    off_t m_fileSize;
    io_service m_ioservice;
    posix::stream_descriptor m_stream;
    asio::streambuf m_buf;
    Config& m_config;
};

void tail_loop(int fd, tailfn callback, off_t pos)
{
    auto& config = configInstance();
    char line[64 * 1024] = {0};
    bool keepgoing = true;
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
            if(!config.follow)
                break;
            sleep(1);
        }
        size_t last = 0;
        for (size_t here = 0; here < readSize; here++)
        {
            if(line[here] == '\r' || line[here] == '\n')
            {
                if(config.verbosity > 10)
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

void tail(const string& filename, tailfn callback)
{
    auto& config = configInstance();

    int fd = ::open(filename.c_str(), O_RDONLY | O_BINARY);
    if (fd < 0)
    {
        cerr << "Error opening file: [" << errno << "] " << filename << endl;
        return;
    }

    off_t pos = config.history ? 0 : fdSize(fd);
    if(pos == (off_t)-1)
    {
        cerr << "Error checking file sile: [" << errno << "] " << filename << endl;
        return;
    }
    if(config.asio)
    {
        TailAsio ta(fd, callback, pos);
        ta.run();
    }
    else
    {
        tail_loop(fd, callback, pos);
    }
}
