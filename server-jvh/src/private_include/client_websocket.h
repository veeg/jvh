#ifndef JVH_CLIENT_WEBSOCKET_H
#define JVH_CLIENT_WEBSOCKET_H

#include "client.h"

namespace jvh
{
    class ClientWebSocket : public Client
    {
    public:
        ClientWebSocket (Server *server, noPollConn *ws);
        ~ClientWebSocket ();

        virtual void send_outgoing_message (const videostream::ToClient& message);

        virtual void read_incoming_message ();

    private:
        noPollConn *m_nopoll_websocket;
        fd_set readfds;

        virtual bool incoming_timeout (unsigned int m_sec);
    };
}

#endif
