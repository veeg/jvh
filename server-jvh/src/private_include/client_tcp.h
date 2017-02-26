#ifndef JVH_CLIENT_TCP_H
#define JVH_CLIENT_TCP_H

#include "client.h"
#include "prototcp.h"


namespace jvh
{
    class ClientTCP : public Client
    {
    public:
        ClientTCP (Server *server, int socket);
        ~ClientTCP ();

        virtual void send_outgoing_message (const videostream::ToClient& message);

        virtual void read_incoming_message ();

    private:
        int m_socket;
        fd_set readfds;
        ProtoTCP *m_proto_tcp;

        virtual bool incoming_timeout (unsigned int);
        int read_from_socket (int read_size, char *buffer);
    };
}

#endif
