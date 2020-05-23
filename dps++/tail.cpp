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

class IO
{
public:
    IO(const string_view& filename, const TailCallBack& callback)
        :m_filename(filename)
        ,m_fd(-1)
        ,m_pos(0)
        ,m_callback(callback)
        ,m_config(Config::instance())
        ,m_keepgoing(true)
    {
        m_fd = ::open(m_filename.data(), O_RDONLY | O_BINARY);
        if (m_fd < 0)
        {
            cerr << "Error opening file: [" << errno << "] " << m_filename << endl;
            throw "TODO: Better throw for opening a file";
        }

        m_fileSize = fdSize();
        m_pos = m_config.history ? 0 : m_fileSize;
        ::lseek(m_fd, m_pos, SEEK_SET);
    }

    off_t parseBuf(const string_view& bufView)
    {
        size_t last = 0;
        for (size_t here = 0; (here < bufView.size() && m_keepgoing); here++)
        {
            if(bufView[here] == '\r' || bufView[here] == '\n')
            {
                if(m_config.verbosity > 10)
                {
                    cout << "Found an end of line character '" << std::hex << bufView[here] << "' here:" << here << ", last:" << last << endl;
                }
                // If (last == here), then we have a bare newline character
                if(last < here)
                {
                    auto line = bufView.substr(last, here - last);
                    m_keepgoing = m_callback(line);
                }
                // Advance last to the next character after.
                last = here + 1;
            }
        }
        return last;
    }

    virtual ~IO()
    {
        if(m_fd >= 0)
            close(m_fd);
    }
protected:
    off_t fdSize()
    {
        struct stat buf;
        int err = fstat(m_fd, &buf);
        if (err < 0)
        {
            cerr << "Error checking file sile: [" << errno << "] " << m_filename << endl;
            throw "TODO: Better throw for fdSize";
        }
        return buf.st_size;
    }

    string_view m_filename;
    int m_fd;
    off_t m_fileSize;
    off_t m_pos;
    const TailCallBack& m_callback;
    Config& m_config;
    bool m_keepgoing;
    static const size_t bufSize = 64 * 1024;
};

class LoopIO : public IO
{
public:
    LoopIO(const string_view& filename, const TailCallBack& callback)
        :IO(filename, callback)
    {
    }

    void run()
    {
        char buf[bufSize] = {0};
        while(m_keepgoing)
        {
            m_fileSize = fdSize();
            off_t newPos = lseek(m_fd, m_pos, SEEK_SET);
            if(newPos > m_fileSize)
            {
                m_pos = m_fileSize;
                cerr << "File truncation detected. New size: " << m_pos << endl;
            }
            off_t readSize = read(m_fd, &buf, sizeof(buf));
            if(readSize == 0)
            {
                if(!m_config.follow)
                    return;
                sleep(1);
            }
            else
            {
                string_view bufView{buf, static_cast<size_t>(readSize)};
                off_t parsedBytes = parseBuf(bufView);
                m_pos += parsedBytes;
            }
        }
    }
private:
};

class ASIO : public IO
{
public:
    ASIO(const string_view& filename, const TailCallBack& callback)
        :IO(filename, callback)
        ,m_stream{m_ioservice, m_fd}
        ,m_buf{bufSize}
    {
    }

    void read_handler(const std::error_code& error, const long unsigned int& readSize)
    {
        off_t parsedBytes = 0;

        if(readSize == 0)
        {
            if(m_buf.size())
            {
                cerr << "Still bytes left" << endl;
            }
            if(!m_config.follow)
                return;
            // TODO: Have a wait_handler for config.follow
            sleep(1);
        }
        else
        {
            string_view bufView{asio::buffer_cast<const char*>(m_buf.data()), m_buf.size()};
            parsedBytes = parseBuf(bufView);
            m_pos += parsedBytes;
            m_buf.consume(parsedBytes);
        }

        if(m_keepgoing)
        {
            asio::async_read(m_stream, m_buf, std::bind(&ASIO::read_handler, this, std::placeholders::_1, std::placeholders::_2));
        }
        else
        {
            m_ioservice.stop();
        }
    }

    void run()
    {
        asio::async_read(m_stream, m_buf, std::bind(&ASIO::read_handler, this, std::placeholders::_1, std::placeholders::_2));
        m_ioservice.run();
        cout << "Done running" << endl;
    }

private:
    io_service m_ioservice;
    posix::stream_descriptor m_stream;
    asio::streambuf m_buf;
};

void tail(const string_view& filename, const TailCallBack& callback)
{
    switch(Config::instance().io)
    {
    case(Config::IO_ASIO):
    {
        ASIO io(filename, callback);
        io.run();
        break;
    }
    case(Config::IO_LOOP):
    default:
    {
        LoopIO io(filename, callback);
        io.run();
        break;
    }
    }
}
