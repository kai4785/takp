#include <vector>
#include <future>
#include <atomic>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <signal.h>

class Context
{
public:
    Context();
    ~Context();
    static Context& instance();
    static void sig_handler(int signal);
    bool set_client(int ordinal, int key, int modifiers);
    void start();
    void wait();
    void cancel();

private:
    struct Client
    {
        int64_t ordinal;
        int32_t keycode;
        int32_t modifiers;
    };

    bool process_keycode(int keycode, int state);
    bool grab(Client hotkey);
    Display*                 m_display;
    Window                   m_window;
    std::array<Client, 3>    m_clients;
    Bool                     m_owner_events    = False;
    int                      m_pointer_mode    = GrabModeAsync;
    int                      m_keyboard_mode   = GrabModeAsync;
    std::future<void>        m_runner;
    std::future<void>        m_watcher;
    std::promise<void>       m_cancel_barrier;
    std::atomic_bool         m_cancelled;
};