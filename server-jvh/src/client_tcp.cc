#include "client_tcp.h"

ClientTCP::ClientTCP (Server *server, int socket_fd) :
    Client (server)
{
    m_sock_filedes = socket_fd;

    FD_ZERO (&readfds);
    FD_SET (m_sock_filedes, &readfds);
}

ClientTCP::~ClientTCP ()
{
    close (m_sock_filedes);
}

void
ClientTCP::send_outgoing_message (const videostream:: ToClient& message)
{
    std::string str;

    message.SerializeToString (&str);
    if (write (m_socket, str.c_str (), str.length ()) < 1)
    {
        throw std::runtime_error (strerror (errno));
    }
}

ClientTCP::read_msg ()
{

}
