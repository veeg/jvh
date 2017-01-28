#ifndef JVH_CLIENT_H
#define JVH_CLIENT_H

#include <nopoll.h>
#include <videostream.pb.h>
#include <sigc++/signal.h>
#include <glibmm/main.h>
#include <thread>
#include "stream.h"

namespace jvh
{
    class Server;

    class Client
    {
    public:
        Client (Server *server);
        virtual ~Client () {
            if (m_stream)
            {
                m_stream->unsubscribe (this);
            }

            teardown_thread = true;
            if (m_client_thread.joinable ()) m_client_thread.join ();
        }

        void listen_async () {
            m_client_thread = std::thread (&Client::handle_traffic, this);
        }

        virtual void send_outgoing_message (const videostream::ToClient& message) = 0;

        void handle_incoming_message (const videostream::FromClient& message);

        virtual void read_incoming_message () = 0;

        sigc::signal<void, Client*>& signal_disconnected ()
        {
            return m_signal_disconnected;
        }

    protected:
        sigc::signal<void, Client*> m_signal_disconnected;

        std::shared_ptr<Stream> m_stream;
        std::shared_ptr<StreamQueue> m_stream_queue;
        std::thread m_client_thread;

        bool teardown_thread = false;
        virtual bool incoming_timeout (unsigned int m_sec) = 0;

        Server *m_server;
        void handle_traffic ();
        bool pending_receive ();
        bool on_hung_up ();
    };
}

#endif
