#ifndef INCLUDE_WINDOW_H
#define INCLUDE_WINDOW_H

#include <X11/Xlib.h>

#include <string>

class XWindow
{
public:
    struct Geometry
    {
        int x;
        int y;
        int w;
        int h;
    };
    XWindow(Display *display, Window window);
    operator Window() { return m_window; };
    Window window() { return m_window; };
    const std::string &name() const;
    bool isActive() const;
    void raise(Time time) const;
    void recenterMouse() const;
private:
    Display *m_display;
    Window m_window;
    std::string m_name;
};

#endif // INCLUDE_WINDOW_H
