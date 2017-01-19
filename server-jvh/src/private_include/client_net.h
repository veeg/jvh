#ifndef JVH_CLIENT_NET_H
#define JVH_CLIENT_NET_H

namespace jvh
{
    class ClientTCP : public client
    {
        ClientTCP (Server *server, int sock_filedes);
        ~ClientTCP ();

    private:
        int m_sock_filedes;
        fd_set readfds;
    };
}

#endif
