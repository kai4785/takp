#ifndef INCLUDE_DISPLAY_H
#define INCLUDE_DISPLAY_H

#include "window.h"

#include <X11/Xlib.h>

#include <string>

class XDisplay
{
public:
    XDisplay();
    ~XDisplay();
    operator Display*() { return m_display; }
    Window root() const;
    XWindow findWindow(const std::string name) const;
    XWindow getInputFocus() const;
private:
    Display *m_display;
    Window m_root;
};

#endif // INCLUDE_DISPLAY_H
