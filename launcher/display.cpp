#include "window.h"
#include "display.h"

#include <memory>
#include <iostream>

struct XBuf
{
    XBuf(unsigned char **buf) : m_buf(buf) {}
    ~XBuf() { XFree(*m_buf); }
    unsigned char **m_buf;
};

XDisplay::XDisplay()
    : m_display(XOpenDisplay(nullptr))
{
    if(!m_display)
        throw std::runtime_error("Display failed to open");
    m_root = XDefaultRootWindow(m_display);
}

XDisplay::~XDisplay()
{
    XCloseDisplay(m_display);
}

Window XDisplay::root() const
{
    return m_root;
}

XWindow XDisplay::findWindow(const std::string windowName) const
{
    Atom actualType;
    int format;
    unsigned long numItems, bytesAfter;
    Window* windows = nullptr;
    XBuf xbuf((unsigned char**)&windows);

    int status = XGetWindowProperty(m_display,
        m_root,
        XInternAtom(m_display, "_NET_CLIENT_LIST" , true),
        0L,
        (~0L),
        false,
        AnyPropertyType,
        &actualType,
        &format,
        &numItems,
        &bytesAfter,
        xbuf.m_buf);

    if(status >= Success && numItems)
    {
        if (format != 32)
        {
            std::cerr << "Format != 32 (" << format << ")" << std::endl;
        }
        for (size_t i = 0; i < numItems; i++)
        {
            XWindow window(m_display, windows[i]);
            if(window.name() == windowName)
            {
                return window;
                break;
            }
        }
    }
    // I don't think I like this as a sential value. Maybe std::optional?
    return XWindow(m_display, m_root);
}

XWindow XDisplay::getInputFocus() const
{
    Window window;
    int revert;
    XGetInputFocus(m_display, &window, &revert);
    return XWindow(m_display, window);
}
