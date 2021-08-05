#include "context.h"
#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;

Context::Context()
    : m_display(XOpenDisplay(nullptr))
    , m_window(DefaultRootWindow(m_display))
    , m_cancelled(false)
{
}

Context::~Context()
{
    for(auto& client : m_clients)
    {
        cout << "XUngrabKey: " << client.keycode << endl;
        XUngrabKey(m_display, client.keycode, client.modifiers, m_window);
    }
    cout << "Closing display" << endl;
    XCloseDisplay(m_display);
}

Context& Context::instance()
{
    static Context context;
    return context;
}

void Context::sig_handler(int signal)
{
    cout << "Signal handler: " << signal << endl;
    Context::instance().cancel();
}

bool Context::set_client(int ordinal, int key, int modifiers)
{
    auto& client = m_clients[ordinal - 1];
    client.ordinal = ordinal;
    client.keycode = XKeysymToKeycode(m_display, key);
    client.modifiers = modifiers;
    return grab(client);
}

bool Context::grab(Client client)
{
    cout << "Registering hotkey: " << client.keycode << ":" << client.modifiers << endl;
    (void)XGrabKey(m_display, client.keycode, client.modifiers | Mod2Mask, m_window, m_owner_events, m_pointer_mode, m_keyboard_mode);
    auto result = XGrabKey(m_display, client.keycode, client.modifiers, m_window, m_owner_events, m_pointer_mode, m_keyboard_mode);
    cout << "XGrabKey: " << client.keycode << " result: " << result << endl;
    if(!result)
        throw std::runtime_error("Bad news!");
    return !!result;
}

int x_error_handler(Display* display, XErrorEvent* event)
{
    auto buf = new char[256];
    XGetErrorText(display, event->error_code, buf, 256);
    cout << "XError: " << event->error_code << ": " << buf << endl;
    delete [] buf;
    Context::instance().cancel();
    return 0;
}

void Context::start()
{
    m_watcher = std::async(std::launch::async, [this] {
        cout << "Watcher started" << endl;
        auto barrier_future = m_cancel_barrier.get_future();
        barrier_future.wait();
        cout << "Watcher done waiting" << endl;
        if(m_cancelled)
        {
            // Send a dummy KeyPress of 0 to wake up XNextEvent
            XKeyEvent event{0};
            event.type = KeyPress;
            event.keycode = 0;
            event.display = m_display;
            event.root = m_window;
            XSendEvent(m_display, m_window, true, KeyPressMask, (XEvent*)&event);
            XFlush(m_display);     // make event happen immediately
        }
    });
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    XSetErrorHandler(x_error_handler);
    m_runner = std::async(std::launch::async, [this] {
        XEvent ev;
        XSelectInput(m_display, m_window, KeyPressMask);
        bool keepgoing = true;
        try
        {
            while(keepgoing)
            {
                auto result = XNextEvent(m_display, &ev);
                cout << "XNextEvent result: " << result << endl;
            
                if(!keepgoing)
                    break;

                switch(ev.type)
                {
                    case KeyPress:
                        keepgoing = process_keycode(ev.xkey.keycode, ev.xkey.state);
                        break;
                    default:
                        break;
                }
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            keepgoing = false;
        }
    });
}

bool Context::process_keycode(int keycode, int state)
{
    switch(keycode)
    {
        case 0:
            cout << "Special keycode 0, all done" << endl;
            return false;
            break;
        default:
            cout << "Hot key pressed! " << keycode << ":" << state << endl;
            auto modifiers = state & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask);
            for(auto& client : m_clients)
            {
                if (keycode == client.keycode && modifiers == client.modifiers)
                {
                    cout << "Found Client: " << client.ordinal << endl;
                    auto command = std::string("/home/kai/.workspace/takp/launcher.sh activate ") + to_string(client.ordinal);
                    cout << command << endl;
                    system(command.c_str());
                }
            }
            break;
    }
    return true;
}

void Context::wait()
{
    m_runner.wait();
}

void Context::cancel()
{
    if(!m_cancelled.exchange(true))
    {
        m_cancel_barrier.set_value();
    }
}