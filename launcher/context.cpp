#include "context.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <map>
#include <thread>
#include <chrono>

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <cstring>
#include <cmath>

using namespace std;

void Context::Client::close_window()
{
    if(window)
    {
        XSelectInput(window->display(), window->window(), NoEventMask);
    }
    window = std::nullopt;
}

void Context::Client::reset()
{
    close_window();
    name.clear();
    keycode = 0;
    modifiers = 0;
    autofire = false;
}

void Context::Autofire::reset()
{
    keycode = 0;
    modifiers = 0;
    keypress =0;
}

Context::Context()
    : m_display()
    , m_running(true)
{
    m_autofire.running.store(false);
}

Context::~Context()
{
    for(auto& client : m_clients)
    {
        client.reset();
    }
    m_autofire.reset();
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

void Context::set_client(int ordinal, const std::string &name, int key, int modifiers)
{
    cout << "set_client" << ordinal << "," << name << "," << key << "," << modifiers << endl;
    auto& client = m_clients[ordinal];
    if(client.ordinal == ordinal &&
       client.name == name &&
       client.keycode == XKeysymToKeycode(m_display, key) &&
       client.modifiers == modifiers)
        return;
    client.reset();
    ungrab(client.keycode, client.modifiers);
    client.ordinal = ordinal;
    client.name = name;
    client.keycode = XKeysymToKeycode(m_display, key);
    client.modifiers = modifiers;
    client.autofire = false;
    grab(client.keycode, client.modifiers);
    client.window = m_display.findWindow(client.name);
}

bool Context::set_autofire(Client& client, int key, int modifiers, int keypress, float delay, float haste, float quiver_haste, int32_t ping_delay)
{
    client.autofire = true;
    cout << "set_autofire " << client.name << "," << key << "," << modifiers << "," << keypress << "," << delay << "," << haste << endl;
    m_autofire.ordinal = client.ordinal;
#if 0
    m_autofire.button_delay = std::trunc((delay*100)/(1+(haste+quiver_haste)/100));
    // https://www.takproject.net/forums/index.php?threads/3-17-2021.18542/#post-94389
    if(m_autofire.button_delay < 1000)
        m_autofire.button_delay = std::trunc((delay*100)/(1+haste/100));
#else
    m_autofire.button_delay = std::trunc((delay*100)/(1+haste/100)*(1.0-quiver_haste/100));
    // https://www.takproject.net/forums/index.php?threads/3-17-2021.18542/#post-94389
    if(m_autofire.button_delay < 1000)
        m_autofire.button_delay = std::trunc((delay*100)/(1+haste/100));
#endif
    m_autofire.ping_delay = ping_delay;
    cout << "Button delay: " << m_autofire.button_delay << " ms" << endl;
    if(m_autofire.keycode == XKeysymToKeycode(m_display, key) &&
       m_autofire.modifiers == modifiers &&
       m_autofire.keypress == XKeysymToKeycode(m_display, keypress))
    {
        return true;
    }
    if(m_autofire.keycode)
    {
        m_autofire.reset();
        ungrab(m_autofire.keycode, m_autofire.modifiers);
    }
    m_autofire.keycode = XKeysymToKeycode(m_display, key);
    m_autofire.modifiers = modifiers;
    m_autofire.keypress = XKeysymToKeycode(m_display, keypress);
    return grab(m_autofire.keycode, m_autofire.modifiers);
}

bool Context::grab(int keycode, int modifiers)
{
    cout << "Registering hotkey: " << keycode << ":" << modifiers << endl;
    auto result = XGrabKey(m_display, keycode, modifiers, m_display.root(), m_owner_events, m_pointer_mode, m_keyboard_mode);
    cout << "XGrabKey: " << keycode << " result: " << result << endl;
    if(!result)
        throw std::runtime_error("Bad news!");
    return !!result;
}

void Context::ungrab(int keycode, int modifiers)
{
    if(!keycode)
        return;
    cout << "XUngrabKey: " << keycode << ":" << modifiers << endl;
    XUngrabKey(m_display, keycode, modifiers, m_display.root());
}

int x_error_handler(Display* display, XErrorEvent* event)
{
    auto buf = new char[256];
    XGetErrorText(display, event->error_code, buf, 256);
    cout << "XError: " << event->resourceid << " : " << event->error_code << ": " << buf << endl;
    delete [] buf;
    Context::instance().bad_window(event->resourceid);
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

    event.type = KeyRelease;
    XSendEvent(m_display, m_display.root(), true, KeyReleaseMask, (XEvent*)&event);
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
    XEvent ev;

    // I'm undecided on whether or not I like Press or Release, or some combination of both
    XSelectInput(m_display, m_display.root(), KeyPressMask | KeyReleaseMask);
    //XSelectInput(m_display, m_display.root(), KeyPressMask);
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
                    cout << "KeyPress: " << ev.xkey.keycode << ":" << ev.xkey.state << endl;
                    //XUngrabKeyboard(m_display, ev.xkey.time);
                    keyPress(ev.xkey.keycode, ev.xkey.state, ev.xkey.time);
                    //XFlush(m_display);
                    break;
                case KeyRelease:
                {
                    cout << "KeyRelease: " << ev.xkey.keycode << ":" << ev.xkey.state << endl;
                    //XUngrabKeyboard(m_display, ev.xkey.time);
                    keyRelease(ev.xkey.keycode, ev.xkey.state, ev.xkey.time);
                    //XFlush(m_display);
                }
                case FocusIn:
                {
                    cout << "Focus in: " << ev.xfocus.window << endl;
                    //auto client = client_from_window(ev.xfocus.window);
                    //if(client)
                        //cout << "  Focus In! " << client->name << endl;
                    break;
                }
                case FocusOut:
                {
                    cout << "Focus out: " << ev.xfocus.window << endl;
                    auto client = client_from_window(ev.xfocus.window);
                    if(client && client->autofire && !client->window->hasInputFocus())
                    {
                        stop_autofire();
                    }
                    if(client)
                    {
                        Window focusedWindow = 0;
                        int focus_state = 0;
                        XGetInputFocus(m_display, &focusedWindow, &focus_state);
                        XWindow newWindow(m_display, focusedWindow);
                        if(newWindow != client->window->window())
                            cout << " Focus switched from client: '" << client->name << "' to window: '" << newWindow.name() << "'" << endl;
                    }
                    break;
                }
                case DestroyNotify:
                case UnmapNotify:
                {
                    bad_window(ev.xdestroywindow.window);
                    break;
                }
                case MappingNotify:
                case ConfigureNotify:
                    // Ignored
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
        m_running.store(false);
    }
}

void Context::autofire_thread()
{
    try
    {
        while(m_running.load())
        {
            std::unique_lock lock(m_autofire.mutex);
            m_autofire.cv.wait(lock);
            auto delay = std::chrono::milliseconds(m_autofire.button_delay);
            auto ping_delay = std::chrono::milliseconds(m_autofire.ping_delay);
            auto mash_delay = std::chrono::milliseconds(20); // 50 FPS is a frame every 20ms
            auto focus_delay = std::chrono::milliseconds(100); // Wait for the window manager to switch focus.
            auto& window = m_clients[m_autofire.ordinal].window;
            auto startTime = std::chrono::high_resolution_clock::now();
            auto endTime = std::chrono::high_resolution_clock::now();

            cout << "  Starting autofire! " << delay.count() << "ms + " << ping_delay.count() << "ms" << endl;
            // Wait 100ms time to allow XSelect a little time to switch input focus back to the client.
            m_autofire.cv.wait_for(lock, focus_delay);
            while(m_autofire.running.load())
            {
                if(!window)
                    break;
                endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
                startTime = endTime;
                cout << "  Pew! " << window->name() << " " << duration << "ms/" << m_autofire.button_delay << "ms" << endl;
                for(int mashing_time = 0; mashing_time <= ping_delay.count(); mashing_time += mash_delay.count() && m_autofire.running.load())
                {
                    XKeyEvent event{0};
                    event.type = KeyPress;
                    event.keycode = m_autofire.keypress;
                    event.display = m_display;
                    event.root = m_display.root();
                    XSendEvent(m_display, window->window(), true, KeyPressMask, (XEvent*)&event);
                    XFlush(m_display);     // make event happen immediately

                    event.type = KeyRelease;
                    XSendEvent(m_display, window->window(), true, KeyReleaseMask, (XEvent*)&event);
                    XFlush(m_display);     // make event happen immediately

                    mashing_time += mash_delay.count();
                    std::this_thread::sleep_for(mash_delay);
                }
                //cout << "Sleeping for " << m_autofire.button_delay << "ms" << endl;
                m_autofire.cv.wait_for(lock, delay);
            }
            cout << "  Stopping autofire!" << endl;
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

    m_autofire_future = std::async(std::launch::async, &Context::autofire_thread, this);

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    XSetErrorHandler(x_error_handler);

    m_event_future = std::async(std::launch::async, &Context::kb_event_thread, this);
}

void Context::keyPress(int keycode, int state, Time time)
{
    if(keycode == 0)
    {
        cout << "Special keycode 0" << endl;
        parse_config();
        return;
    }
    auto modifiers = state & 0x7F; // 127
    //cout << "Hot key pressed! " << keycode << ":" << modifiers << "(" << state << ")" << endl;

    if(keycode == m_autofire.keycode && modifiers == m_autofire.modifiers)
    {
        auto& window = m_clients[m_autofire.ordinal].window;
        if(!window)
        {
            cout << " ** Autofire window not set. skipping." << endl;
            return;
        }
        if(!window->hasInputFocus())
        {
            cout << " ** Autofire window not active. skipping." << endl;
            stop_autofire();
            return;
        }
        m_autofire.running.exchange(!m_autofire.running.load());
        m_autofire.cv.notify_all();
        return;
    }

    auto client = client_from_key(keycode, modifiers);
    if(!client)
        return;

    if(client->window)
    {
        if(client->autofire)
            m_autofire.ordinal = client->ordinal;
        else
            stop_autofire();

		if(!client->window->isActive())
        {
            cout << "  Raising client: '" << client->name << "' (" << client->window->window() << ")" << endl;
			client->window->raise(time);
        }
        else
        {
            client->window->recenterMouse();
        }

        return;
    }

}

void Context::keyRelease(int keycode, int state, Time time)
{
    if(keycode == 0)
    {
        return;
    }

    auto modifiers = state & 0x7F; // 127

    if(keycode == m_autofire.keycode && modifiers == m_autofire.modifiers)
        return;

    auto client = client_from_key(keycode, modifiers);
    if(!client)
        return;

    if(client->window)
        cout << " KeyRelease refreshing Window: " << client->name << " (" << client->window->window() << ")" << endl;
    else
        cout << " KeyRelease refreshing Window: " << client->name << " (NONE)" << endl;
    client->window = m_display.findWindow(client->name);
    if(client->window)
    {
        cout << " KeyRelease refreshed Window: " << client->name << " (" << client->window->window() << ")" << endl;
        XSelectInput(m_display, client->window->window(), FocusChangeMask | SubstructureNotifyMask);
    }
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
        stop_autofire(true);
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
    vector<string> autofire;
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
        if(key == "autofire")
        {
            autofire.clear();
            parse_config_array(value, autofire);
            cout << "Autofire found: ";
            for(auto& v : autofire)
                cout << v << " ";
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
        if(autofire[0] == active_accounts[i] && keys.size() >= 4)
        {
            cout << "Autofire key: (" << keys[3] << "|" << modifiers[3] << ")" << endl;
            cout << "Autofire keypress: (" << keys[4] << ")" << endl;
            set_autofire(m_clients[i], keys[3], modifiers[3], keys[4],
                std::stoi(autofire[1]),
                std::stoi(autofire[2]),
                std::stoi(autofire[3]),
                std::stoi(autofire[4])
            );
        }
    }
}

void Context::stop_autofire(bool force)
{
    if(m_autofire.running.exchange(false) || force)
    {
        m_autofire.cv.notify_all();
    }
}

void Context::bad_window(Window window)
{
    auto client = client_from_window(window);
    if(client && client->window)
    {
        cout << "  Window closed! " << client->name << ", " << window << endl;
        client->close_window();
    }
}

Context::Client* Context::client_from_window(Window window)
{
    for(auto &client : m_clients)
    {
        if(client.window && client.window->window() == window)
            return &client;
    }
    return nullptr;
}

Context::Client* Context::client_from_key(int keycode, int modifiers)
{
    for(auto& client : m_clients)
    {
        if (keycode == client.keycode && modifiers == client.modifiers)
            return &client;
    }
    return nullptr;
}
