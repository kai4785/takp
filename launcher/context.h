#ifndef INCLUDE_CONTEXT_H
#define INCLUDE_CONTEXT_H

#include "display.h"
#include "window.h"

#include <vector>
#include <future>
#include <atomic>
#include <map>
#include <string>
#include <filesystem>
#include <mutex>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <signal.h>

struct Account
{
    std::string user;
    std::string pass;
};

// TODO: This is too big. We do at least 5 things that may be separated
// 1) Raise an X window to the foreground, and move the mouse
// 2) Keyboard event monitor (to Raise an X window)
// 3) Config file parser (to configure Keyboard event monitor)
// 4) inotify (to detect when Config is changed)
// 5) autofire
class Context
{
private:
    using Accounts = std::map<std::string, Account>;
    struct Client
    {
        int64_t ordinal;
        std::optional<XWindow> window;
        std::string name;
        int32_t keycode;
        int32_t modifiers;
        bool autofire;

        void close_window();
        void reset();
    };

    struct Autofire
    {
        int64_t ordinal;
        int32_t keycode;
        int32_t modifiers;
        int32_t keypress;
        int32_t button_delay;
        int32_t ping_delay;
        std::atomic_bool running;
        std::condition_variable cv;
        std::mutex mutex;

        void reset();
    };

public:
    Context();
    ~Context();
    static Context& instance();
    static void sig_handler(int signal);
    void set_client(int ordinal, const std::string& name, int key, int modifiers);
    bool set_autofire(Client& client, int key, int modifiers, int keypress, float delay, float haste, float quiver_haste, int32_t ping_delay);
    void unset_clients();
    void start();
    void wait();
    void cancel();
    void set_config_file(const std::filesystem::path& config_file);
    void parse_config();
    void stop_autofire(bool force = false);
    void bad_window(Window);

private:
    void keyPress(int keycode, int state, Time time);
    void keyRelease(int keycode, int state, Time time);
    bool grab(int keycode, int modifiers);
    void ungrab(int keycode, int modifiers);
    void read_inotify(int fd, int wd);
    void dummy_keypress();
    void cancel_thread();
    void inotify_thread();
    void kb_event_thread();
    void autofire_thread();
    void parse_config_value(std::string& value);
    void parse_config_array(std::string& value, std::vector<std::string>& values, char delimiter = ' ');
    void parse_config_strip(std::string& value);
    Client* client_from_window(Window window);
    Client* client_from_key(int keycode, int modifiers);
    XDisplay                 m_display;
    std::array<Client, 3>    m_clients;
    Autofire                 m_autofire;
    Bool                     m_owner_events    = False;
    int                      m_pointer_mode    = GrabModeAsync;
    int                      m_keyboard_mode   = GrabModeAsync;
    int                      m_inotify_pipe[2];
    std::future<void>        m_event_future;
    std::future<void>        m_inotify_future;
    std::future<void>        m_cancel_future;
    std::future<void>        m_autofire_future;
    std::promise<void>       m_cancel_barrier;
    std::atomic_bool         m_running;
    std::filesystem::path    m_inotify_path;
    std::filesystem::path    m_config_file;
    Accounts                 m_accounts;
};

#endif // INCLUDE_CONTEXT_H
