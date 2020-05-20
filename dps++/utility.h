#ifndef UTILITY_H
#define UTILITY_H

#include <cstdint>
#include <string>
#include <string_view>

bool fromInt(char* data, size_t length, int64_t value);
int64_t toInt(const std::string_view& str);
bool endsWith(const std::string_view& str, const std::string_view& end);
bool startsWith(const std::string_view& str, const std::string_view& end);

// Simple class to help pass around a buffer to mutable data
class StringBuf
{
public:
    StringBuf(size_t size, char* data = nullptr)
        :m_size(size)
        ,m_data(data)
        ,m_owner(false)
    {
        if(m_data == nullptr)
        {
            m_data = new char[size];
            m_owner = true;
        }
    }

    ~StringBuf()
    {
        if(m_owner)
        {
            delete[] m_data;
        }
    }

    char* data()
    {
        return m_data;
    }

    size_t size()
    {
        return m_size;
    }

    char& operator[](size_t i)
    {
        return m_data[i];
    }

    const char& operator[](std::size_t i) const
    {
        return m_data[i];
    }

    bool fromInt(size_t pos, size_t length, int64_t value)
    {
        return ::fromInt(&m_data[pos], length, value);
    }

    // TODO: C++ify
    void assign(const std::string_view& str, size_t pos, size_t length)
    {
        for(int i = 0; i < str.size(); i++)
        {
            m_data[pos + i] = str[i];
        }
    }

    std::string_view string_view()
    {
        return std::string_view(m_data, m_size);
    }

    operator std::string_view()
    {
        return std::string_view(m_data, m_size);
    }

    operator std::string()
    {
        return std::string(m_data, m_size);
    }

private:
    size_t m_size;
    char* m_data;
    bool m_owner;
};

#endif // UTILITY_H
