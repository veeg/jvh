#include "client_tcp.h"

using namespace jvh;

ClientTCP::ClientTCP (Server *server, int socket_fd) :
    Client (server)
{
    m_socket = socket_fd;

    FD_ZERO (&readfds);
    FD_SET (m_socket, &readfds);
}

ClientTCP::~ClientTCP ()
{
    close (m_socket);
}

void
ClientTCP::send_outgoing_message (const videostream::ToClient& message)
{
    std::string str;

    message.SerializeToString (&str);

    if (write (m_socket, (const void *) str.length (), sizeof (int32_t)))
    {
        on_hung_up ();
    }

    if (write (m_socket, str.c_str (), str.length ()) < 0)
    {
        on_hung_up ();
    }
}

bool
ClientTCP::incoming_timeout (unsigned int m_sec)
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
ClientTCP::read_incoming_message ()
{
    uint32_t message_size;

    if (read_from_socket (sizeof (uint32_t), (char*)&message_size) < 0)
    {
        on_hung_up ();
    }

    char *message = new char[message_size];

    if (read_from_socket (message_size, message) < 0)
    {
        on_hung_up ();
    }

    videostream::FromClient pbmessage;
    pbmessage.ParseFromString ((const char *)  message);

    handle_incoming_message (pbmessage);

    delete message;
}

int
ClientTCP::read_from_socket (int read_size, char *buffer)
{
    int received = 0;
    int bytes_read;

    while (received < read_size)
    {
        bytes_read = read (m_socket, buffer, read_size - received);
        if (bytes_read < 0)
        {
            return -1;
        }

        received += bytes_read;
    }
    return 0;
}
