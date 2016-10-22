#ifndef JVH_CLIENT_H
#define JVH_CLIENT_H

#include <nopoll.h>
#include <videoplay.pb.h>
#include <sigc++/signal.h>
#include <glibmm/main.h>

namespace jvh
{
    class Client
    {
    public:
        Client (noPollConn *ws);
        ~Client();

        void send_outgoing_message (const videoplay::ToClient& message);
        sigc::signal<void, Client*>& signal_disconnected ()
        {
            return m_signal_disconnected;
        }

    private:
        sigc::signal<void, Client*> m_signal_disconnected;
        noPollConn *m_nopoll_websocket;
        bool on_traffic (Glib::IOCondition condition);
        bool on_hung_up (Glib::IOCondition);
    };
}

#endif
