#include "context.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <map>

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <cstring>

using namespace std;

Context::Context()
    : m_display()
    , m_running(true)
{
}

Context::~Context()
{
    for(auto& client : m_clients)
    {
        ungrab(client);
    }
    cout << "Closing display" << endl;
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

bool Context::set_client(int ordinal, const std::string &name, int key, int modifiers)
{
    cout << "set_client" << ordinal << "," << name << "," << key << "," << modifiers << endl;
    auto& client = m_clients[ordinal];
    if(client.ordinal == ordinal &&
       client.name == name &&
       client.keycode == XKeysymToKeycode(m_display, key) &&
       client.modifiers == modifiers)
        return true;
    ungrab(client);
    client.ordinal = ordinal;
    client.name = name;
    client.keycode = XKeysymToKeycode(m_display, key);
    client.modifiers = modifiers;
    return grab(client);
}

bool Context::grab(const Client& client)
{
    cout << "Registering hotkey: " << client.keycode << ":" << client.modifiers << endl;
    auto result = XGrabKey(m_display, client.keycode, client.modifiers, m_display.root(), m_owner_events, m_pointer_mode, m_keyboard_mode);
    cout << "XGrabKey: " << client.keycode << " result: " << result << endl;
    if(!result)
        throw std::runtime_error("Bad news!");
    return !!result;
}

void Context::ungrab(Client& client)
{
    if(!client.keycode)
        return;
    cout << "XUngrabKey: " << client.keycode << ":" << client.modifiers << endl;
    XUngrabKey(m_display, client.keycode, client.modifiers, m_display.root());
    client.name.clear();
    client.keycode = 0;
    client.modifiers = 0;
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

void Context::dummy_keypress()
{
    // Send a dummy KeyPress of 0 to wake up XNextEvent
    XKeyEvent event{0};
    event.type = KeyPress;
    event.keycode = 0;
    event.display = m_display;
    event.root = m_display.root();
    XSendEvent(m_display, m_display.root(), true, KeyPressMask, (XEvent*)&event);
    XFlush(m_display);     // make event happen immediately
}

void Context::cancel_thread()
{
    cout << "Watcher started" << endl;
    auto barrier_future = m_cancel_barrier.get_future();
    barrier_future.wait();
    cout << "Watcher done waiting" << endl;
    if(!m_running.load())
    {
        dummy_keypress();
    }
}

void Context::inotify_thread()
{
    // Mostly ripped right out of the man page
    char buf = '\0';
    int fd = 0, i = 0, poll_num = 0, wd = 0;
    nfds_t nfds;
    struct pollfd fds[2];

    fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1) {
        perror("inotify_init1");
        return;
    }

    m_inotify_path = m_config_file.parent_path();
    wd = inotify_add_watch(fd, m_inotify_path.c_str(), IN_OPEN | IN_CLOSE);
    if (wd == -1) {
        perror("inotify_add_watch");
        return;
    }

    // Self Pipe trick
    if(pipe(m_inotify_pipe) < 0) {
        perror("pipe");
        return;
    }

    nfds = 2;

    fds[0].fd = m_inotify_pipe[0];
    fds[0].events = POLLIN;
    fds[1].fd = fd;
    fds[1].events = POLLIN;

    while (1) {
        poll_num = poll(fds, nfds, -1);
        if (poll_num == -1) {
            if (errno == EINTR)
                continue;
            perror("poll");
            return;
        }
        if (poll_num > 0) {
            if (fds[0].revents & POLLIN) {
                while (read(fds[0].fd, &buf, 1) > 0 && buf != '\n')
                    continue;
                break;
            }

            if (fds[1].revents & POLLIN) {
                read_inotify(fd, wd);
            }
        }
    }

    close(fd);
    close(m_inotify_pipe[0]);
    close(m_inotify_pipe[1]);

    return;
}

void Context::read_inotify(int fd, int wd)
{
    char buf[4096]
        __attribute__ ((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    ssize_t len;

    for (;;) {
        len = read(fd, buf, sizeof(buf));
        if (len == -1 && errno != EAGAIN) {
            perror("read");
            return;
        }

        if(len <= 0)
            break;

        for(char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
            event = (const struct inotify_event *) ptr;
            auto file = m_inotify_path / event->name;
            if(wd == event->wd && file == m_config_file && event->mask & IN_CLOSE_WRITE) {
                cout << "Config file updated: " << file << endl;
                dummy_keypress();
            }
        }
    }
}

void Context::kb_event_thread()
{
    bool consumed_event = false;
    XEvent ev;

    XSelectInput(m_display, m_display.root(), KeyPressMask);
    try
    {
        while(m_running.load())
        {
            auto result = XNextEvent(m_display, &ev);
        
            if(!m_running.load())
                break;

            switch(ev.type)
            {
                case KeyPress:
                    XUngrabKeyboard(m_display, ev.xkey.time);
                    consumed_event = process_keycode(ev.xkey.keycode, ev.xkey.state, ev.xkey.time);
                    XFlush(m_display);
#if 0
                    if(!consumed_event)
                    {
                        //auto window = m_display.getInputFocus();
                        auto window = XWindow(m_display, 0x05400001);
                        cout << "Forwarding event to window: " << window.name() << endl;
                        XSendEvent(m_display, window.window(), true, ev.xkey.type, &ev);
                        XFlush(m_display);
                    }
#endif
                    break;
                default:
                    cout << "  Something else: " << ev.type << endl;
                    break;
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        m_running.exchange(false);
    }
}

void Context::start()
{
    m_cancel_future = std::async(std::launch::async, &Context::cancel_thread, this);

    m_inotify_future = std::async(std::launch::async, &Context::inotify_thread, this);

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    XSetErrorHandler(x_error_handler);

    m_event_future = std::async(std::launch::async, &Context::kb_event_thread, this);
}

bool Context::process_keycode(int keycode, int state, Time time)
{
    switch(keycode)
    {
        case 0:
            cout << "Special keycode 0" << endl;
            parse_config();
            return true;
            break;
        default:
            auto modifiers = state & 0x7F; // 127
            cout << "Hot key pressed! " << keycode << ":" << modifiers << "(" << state << ")" << endl;
            for(auto& client : m_clients)
            {
                if (keycode == client.keycode && modifiers == client.modifiers)
                {
                    XWindow window = m_display.findWindow(client.name);

                    if(window.name() == client.name)
                    {
                        cout << "Found Client Window: " << window.name() << endl;
                        if(window.isActive())
                        {
                            window.recenterMouse();
                        }
                        else
                        {
                            window.raise(time);
                        }
                        return true;
                    }
                    break;
                }
            }
            break;
    }
    return false;
}

void Context::wait()
{
    m_event_future.wait();
}

void Context::cancel()
{
    if(m_running.exchange(false))
    {
        const char nl = '\n';
        write(m_inotify_pipe[1], &nl, 1);
        m_cancel_barrier.set_value();
    }
}

void Context::set_config_file(const filesystem::path& config_file)
{
    m_config_file = config_file;
    parse_config();
}

void Context::parse_config_value(string& value)
{
}

void Context::parse_config_array(std::string& value, vector<string>& values, const char delimiter)
{
    auto openp = value.find('(');
    auto closep = value.find(')');
    if(openp != string::npos && closep != string::npos)
        value = value.substr(openp + 1, closep - 1);

    auto end = string::npos;
    auto start = string::npos;
    start = 0;
    do
    {
        end = value.find(delimiter, start);
        auto name = value.substr(start, end - start);
        parse_config_strip(name);
        values.push_back(name);
        start = end + 1;
    } while (end != string::npos);
}

void Context::parse_config_strip(string& value)
{
    if(value[0] == '"' && value[value.size()-1] == '"')
        value = value.substr(1, value.size()-2);
}

int string_to_modifier(const std::string& value)
{
    static const map<string, int> value_map = {
        {"ShiftMask", ShiftMask},     // Shift
        {"LockMask", LockMask},       // CapsLock
        {"ControlMask", ControlMask}, // Ctrl
        {"Mod1Mask", Mod1Mask},       // Alt
        {"Mod2Mask", Mod2Mask},       // Numlock
        {"Mod3Mask", Mod3Mask},       // Scroll Lock
        {"Mod4Mask", Mod4Mask},       // Super
        {"Mod5Mask", Mod5Mask},       // Unknown?
        {"AnyModifier", AnyModifier}
    };
    auto iter = value_map.find(value);
    if(iter == value_map.end())
        return 0;
    return iter->second;
}

void Context::parse_config()
{
    m_accounts.clear();
    ifstream input(m_config_file);
    vector<string> active_accounts;
    vector<KeySym> keys;
    vector<int> modifiers;
    for(string line; getline(input, line);)
    {
        auto pos = line.find('#');
        if(pos != string::npos)
            line.resize(pos);
        if(!line.size())
            continue;
        pos = line.find('=');
        if(pos == string::npos)
            continue;
        auto key = line.substr(0, pos);
        auto value = line.substr(pos + 1);
        if(key == "accounts")
        {
            // Last one in wins
            active_accounts.clear();
            parse_config_array(value, active_accounts);
            cout << "Accounts found: ";
            for(auto& name : active_accounts)
                cout << name << " ";
            cout << endl;
        }
        if(key == "keys")
        {
            // Last one in wins
            keys.clear();
            vector<string> key_strings;
            parse_config_array(value, key_strings);
            cout << "Keyboard Shortcuts found: ";
            for(auto& shortcut : key_strings) {
                int modifier = 0;
                vector<string> shortcut_strings;
                parse_config_array(shortcut, shortcut_strings, '|');
                auto keysym = XStringToKeysym(shortcut_strings[0].c_str());
                cout << shortcut_strings[0] << "(" << keysym << ")";
                for(int i = 1; i < shortcut_strings.size(); i++) {
                    modifier |= string_to_modifier(shortcut_strings[i]);
                }
                cout << "(" << modifier << ") ";
                keys.push_back(keysym);
                modifiers.push_back(modifier);
            }
            cout << endl;
        }
        // This was nice for Bash, but do we want to keep it here forever?
        string account_ = "account_";
        string _password = "_password";
        string _suffix = "_suffix";
        if(key.substr(0, account_.size()) == account_)
        {
            if(key.substr(key.size() - _suffix.size()) == _suffix)
            {
                string account = key.substr(account_.size(), key.size() - _suffix.size() - account_.size());
                parse_config_strip(value);
            }
            if(key.substr(key.size() - _password.size()) == _password)
            {
                string account = key.substr(account_.size(), key.size() - _password.size() - account_.size());
                parse_config_strip(value);
                m_accounts[account] = {account, value};
            }
        }
    }
    string prefix = "takp-";
    for(int i = 0; i < active_accounts.size() && i < 3; i++)
    {
        string windowName = prefix + active_accounts[i];
        cout << "[" << i << "]" << windowName << endl;
        set_client(i, windowName, keys[i], modifiers[i]);
    }
}
