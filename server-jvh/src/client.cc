#include "client.h"
#include <iostream>


using namespace jvh;


Client::Client (noPollConn *ws) :
    m_nopoll_websocket (ws)
{
}

Client::~Client ()
{
    nopoll_conn_unref (m_nopoll_websocket);
}

    void
Client::send_outgoing_message(const videoplay::ToClient& message)
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
Client::on_traffic (Glib::IOCondition condition)
{
    std::cerr << "on_traffic" << std::endl;
    auto request = nopoll_conn_get_msg (m_nopoll_websocket);

    if (request == nullptr)
    {
        on_hung_up (condition);
        return false;
    }

    videoplay::FromClient message;
    message.ParseFromString ((const char *) nopoll_msg_get_payload (request));

    //handle_incoming_message (message);

    nopoll_msg_unref (request);
    return true;
}



bool
Client::on_hung_up (Glib::IOCondition)
{
    std::cerr << "Disconnecting client trafic" << std::endl;
    m_signal_disconnected (this);
    return false;
}
