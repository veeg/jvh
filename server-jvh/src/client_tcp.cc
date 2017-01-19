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


ClientTCP::traffic_listen ()
{
    if (select (FD_SETSIZE, &readfds, NULL, NULL, NULL) < 0)
    {
        throw std::runtime_error (strerror (errno));
    }
}


ClientTCP::read_msg ()
{

}
