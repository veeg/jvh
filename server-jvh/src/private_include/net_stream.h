#ifndef JVH_STREAM_ENCODE_TCP_H
#define JVH_STREAM_ENCODE_TCP_H

#include "prototcp.h"
#include <sigc++/signal.h>
#include "stream.h"
#include "encoder.h"
#include "server.h"
#include "net_stream.h"

namespace jvh
{
    class Server;
    class NetStream : public Stream
    {

    public:
        NetStream (int socket, Server *server);
        ~NetStream ();

        virtual std::shared_ptr<StreamQueue> subscribe (void *client);

        virtual void unsubscribe (void *client);

        virtual bool is_active () {}

        sigc::signal<void, Stream*>& signal_disconnected ()
        {
            return m_signal_disconnected;
        }
        void run_threaded ()
        {
            m_stream_thread = std::thread (&NetStream::handle_traffic, this);
        }
    protected:
        sigc::signal<void, Stream*> m_signal_disconnected;
        Server *m_server;
        ProtoTCP *m_pbcomm;
        Encoder *m_encoder;
        std::thread m_stream_thread;
        std::mutex m_sublock;
        std::atomic<bool> stream_shutdown;
        std::map<void *, std::shared_ptr<StreamQueue>> m_subscriptions;

        struct stream_entry *m_stream_context;

        //! Callback used by the encoder to send
        //! encoded frames
        void enqueue_to_all_subcriptions (AVPacket *pkt);

        //! Reads the entire Payload form message and copies it to
        //! a uint8_t array
        uint8_t * payload_to_frame (videostream::FromFeed& message);

        bool alloc_encoder (int width, int height, enum AVPixelFormat fmt);

        void handle_incoming_message (videostream::FromFeed& message);

        void read_incoming_message (videostream::FromFeed& message);

        bool on_stream_hung_up ();

        void handle_traffic ();
    };
}

#endif //JVH_STREAM_ENCODE_TCP_H
