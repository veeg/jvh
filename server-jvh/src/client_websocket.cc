#include "client_websocket.h"

ClientWebSocket::ClientWebSocket (Server *server, noPollConn *ws) :
    Client (server)
{
    /*std::cerr << "Constructed new client" << std::endl;
    Glib::signal_io ().connect (sigc::mem_fun (*this, &Client::on_traffic),
                                nopoll_conn_socket (ws),
                                Glib::IO_IN | Glib::IO_HUP);
    */
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
ClientWebSocket::traffic_listen ()
{
    if (select (FD_SETSIZE, &readfds, NULL, NULL, NULL) < 0)
    {
        throw std::runtime_error (strerror (errno));
    }

    auto request = nopoll_conn_get_msg (m_nopoll_websocket);

    if (request == nullptr)
    {
        on_hung_up (condition);
        return false;
    }

    videostream::FromClient message;
    message.ParseFromString ((const char *) nopoll_msg_get_payload (request));

    handle_incoming_message (message);

    nopoll_msg_unref (request);
    return true;
}

