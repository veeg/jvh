#include "client_tcp.h"
#include <iostream>

using namespace jvh;

ClientTCP::ClientTCP (Server *server, int socket) :
    Client (server)
{
    m_proto_tcp = new ProtoTCP (socket);
}

ClientTCP::~ClientTCP ()
{
    delete m_proto_tcp;
}

void
ClientTCP::send_outgoing_message (const videostream::ToClient& message)
{
    if (m_proto_tcp->send (message) == false)
    {
        on_hung_up ();
    }
}

bool
ClientTCP::incoming_timeout (unsigned int m_sec)
{
    return m_proto_tcp->incoming_timeout (m_sec);
}

void
ClientTCP::read_incoming_message ()
{
    videostream::FromClient pbmessage;

    if (m_proto_tcp->receive (pbmessage) == false)
    {
        on_hung_up ();
        return;
    }

    handle_incoming_message (pbmessage);
}


