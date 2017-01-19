#ifndef JVH_CLIENT_WEBSOCKET_H
#define JVH_CLIENT_WEBSOCKET_H

namespace jvh
{
    class ClientWebSocket : public Client
    {
    public:
        ClientWebSocket (Server *server, noPollConn *ws);
        ~ClientWebSocket ();

        virtual void send_outgoing_message (const videostream::ToClient& message);

    private:
        noPollConn *m_nopoll_websocket;

        virtual bool on_traffic (Glib::IOCondition condition);

    };
}

#endif
