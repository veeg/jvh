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
        Client (Server *server);
        ~Client() {
            teardown_thread = true;
            if (m_client_thread.joinable ()) m_client_thread.join ();
        }

        void listen_async () {
            m_client_thread = std::thread (&Client::handle_traffic, this);
        }

        virtual void send_outgoing_message (const videostream::ToClient& message) = 0;
        void handle_incoming_message (const videostream::FromClient& message);
        sigc::signal<void, Client*>& signal_disconnected ()
        {
            return m_signal_disconnected;
        }

    private:
        sigc::signal<void, Client*> m_signal_disconnected;

        std::thread m_client_thread;

        bool teardown_thread = false;

        Server *m_server;
        virtual bool on_traffic (Glib::IOCondition) = 0;
        bool on_hung_up (Glib::IOCondition);
    };
}

#endif
