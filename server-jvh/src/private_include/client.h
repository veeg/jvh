#ifndef JVH_CLIENT_H
#define JVH_CLIENT_H

#include <nopoll.h>
#include <videostream.pb.h>
#include <sigc++/signal.h>
#include <glibmm/main.h>


namespace jvh
{

    class Server;

    class Client
    {
    public:
        Client (Server *server, noPollConn *ws);
        ~Client();

        void send_outgoing_message (const videostream::ToClient& message);
        void handle_incoming_message (const videostream::FromClient& message);
        sigc::signal<void, Client*>& signal_disconnected ()
        {
            return m_signal_disconnected;
        }

    private:
        sigc::signal<void, Client*> m_signal_disconnected;
        noPollConn *m_nopoll_websocket;
        Server *m_server;
        bool on_traffic (Glib::IOCondition condition);
        bool on_hung_up (Glib::IOCondition);
    };
}

#endif
