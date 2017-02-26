#include "include/prototcp.h"
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>

using namespace jvh;

ProtoTCP::ProtoTCP (int socket)
{
    m_socket = socket;

    FD_ZERO (&readfds);
    FD_SET (m_socket, &readfds);
}

ProtoTCP::~ProtoTCP ()
{
    if (m_socket)
    {
        close (m_socket);
    }
}

bool
ProtoTCP::incoming_timeout (unsigned int m_sec)
{
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = m_sec;
    int ret;

    ret = select (FD_SETSIZE, &readfds, NULL, NULL, &timeout);
    if (ret < 0)
    {
        std::cerr << strerror (errno) << std::endl;
    }
    return !(ret == 0);
}

uint32_t
ProtoTCP::read_message_header ()
{
    int ret;
    uint32_t message_size;

    ret = read_from_socket (sizeof (uint32_t), (char*)&message_size);

    return ret < 0 ? ret : message_size;
}

int
ProtoTCP::read_from_socket (uint32_t read_size, char *buffer)
{
    int received = 0;
    int bytes_read;

    while (received < read_size)
    {
        bytes_read = read (m_socket, &buffer[received], read_size - received);
        if (bytes_read <= 0)
        {
            close (m_socket);
            return -1;
        }
        received += bytes_read;
    }
    assert (read_size == received);
    return 0;
}

int
ProtoTCP::write_to_socket (uint32_t write_size, char *buffer)
{
    int sent = 0;
    int bytes_written;

    while (sent < write_size)
    {
        bytes_written = write (m_socket, (char *)&buffer[sent], write_size - sent);
        if (bytes_written <= 0)
        {
            close (m_socket);
            return -1;
        }
        sent += bytes_written;
    }
    assert (write_size == sent);

    return 0;
}
