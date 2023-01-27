#include "window.h"

#include <iostream>

XWindow::XWindow(Display *display, Window window)
    : m_display(display)
    , m_window(window)
{
    Atom actual_type;
    unsigned char *namedata;
    unsigned long namenumItems, namebytesAfter;
    int nameformat;
    if(window == PointerRoot)
    {
        m_name = "PointerRoot";
        return;
    }
    if(window == None)
    {
        m_name = "None";
        return;
    }
    int status = XGetWindowProperty(m_display,
        m_window,
        XInternAtom(m_display, "_NET_WM_NAME", False),
        0L,
        (~0L),
        false,
        AnyPropertyType,
        &actual_type,
        &nameformat,
        &namenumItems,
        &namebytesAfter,
        &namedata);
    if(status == 0 && namenumItems == 0)
    {
        status = XGetWindowProperty(m_display,
            m_window,
            XInternAtom(m_display, "_NET_DESKTOP_NAMES", False),
            0L,
            (~0L),
            false,
            AnyPropertyType,
            &actual_type,
            &nameformat,
            &namenumItems,
            &namebytesAfter,
            &namedata);
    }
    if(status >= Success && namenumItems)
    {
        m_name = (char *)namedata;
    }
    XFree(namedata);
}

const std::string &XWindow::name() const
{
    return m_name;
}

bool XWindow::isActive() const
{
    Window focusedWindow;
    int focus_state;
    XGetInputFocus(m_display, &focusedWindow, &focus_state);
    return m_window == focusedWindow;
}

void XWindow::raise(Time time) const
{
    std::cout << "Raising window!" << std::endl;
    XEvent event = { 0 };
    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.message_type = XInternAtom(m_display, "_NET_ACTIVE_WINDOW", False);
    event.xclient.window = m_window;
    event.xclient.format = 32;
    event.xclient.data.l[0] = 1;
    event.xclient.data.l[1] = time;
    event.xclient.data.l[2] = 0;

    XSendEvent(m_display, XDefaultRootWindow(m_display), False, SubstructureRedirectMask | SubstructureNotifyMask, &event );

    XMapRaised(m_display, m_window);
}

void XWindow::recenterMouse() const
{
    XWindowAttributes attributes;
    XGetWindowAttributes(m_display, m_window, &attributes);
    int newx = attributes.x + attributes.width / 2;
    int newy = attributes.y + attributes.height / 2;
    std::cout << "Moving Mouse! [" << newx << "," << newy << "]" << std::endl;
    XWarpPointer(m_display, None, XDefaultRootWindow(m_display), 0, 0, 0, 0, newx, newy);
}
