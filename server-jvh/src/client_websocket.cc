#include "client_websocket.h"
#include <iostream>

using namespace jvh;

ClientWebSocket::ClientWebSocket (Server *server, noPollConn *ws) :
    Client (server)
{
    if (nopoll_conn_wait_until_connection_ready(m_nopoll_websocket, 10))
    {
        std::cerr << "wait until connection read yay!" << std::endl;
    }

    FD_ZERO (&readfds);
    FD_SET(nopoll_conn_socket (ws), &readfds);
}

ClientWebSocket::~ClientWebSocket ()
{
    nopoll_conn_unref (m_nopoll_websocket);
}

void
ClientWebSocket::send_outgoing_message(const videostream::ToClient& message)
{
    std::string str;

    message.SerializeToString (&str);
    if (nopoll_conn_send_binary (m_nopoll_websocket, str.c_str(), str.size()) == -1
            && errno != NOPOLL_EWOULDBLOCK)
    {
        throw std::runtime_error (strerror(errno));
    }
}

bool
ClientWebSocket::incoming_timeout (unsigned int m_sec)
{
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = m_sec;

    if (select (FD_SETSIZE, &readfds, NULL, NULL, &timeout) < 0)
    {
        return false;
    }

    return true;
}

void
ClientWebSocket::read_incoming_message ()
{
    auto request = nopoll_conn_get_msg (m_nopoll_websocket);
//        on_hung_up (condition);

    if (request == nullptr)
    {
        on_hung_up ();
        return;
    }

    videostream::FromClient message;
    message.ParseFromString ((const char *)nopoll_msg_get_payload (request));
    std::cerr << "Heia" << std::endl;

    handle_incoming_message (message);

    nopoll_msg_unref (request);
}
