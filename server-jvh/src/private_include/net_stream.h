#ifndef JVH_STREAM_ENCODE_TCP_H
#define JVH_STREAM_ENCODE_TCP_H

#include "prototcp.h"
#include <sigc++/signal.h>
#include "stream.h"
#include "encoder.h"

namespace jvh
{
    class Server;
    class NetStream : public Stream
    {
    friend class Server;
    public:
        NetStream (int socket);
        ~NetStream ();

        virtual std::shared_ptr<StreamQueue> subscribe (void *client);

        virtual void unsubscribe (void *client);

        sigc::signal<void, Stream*>& signal_disconnected ()
        {
            return m_signal_disconnected;
        }
    protected:
        sigc::signal<void, Stream*> m_signal_disconnected;
        Server *m_server;
        ProtoTCP *m_pbcomm;
        Encoder *m_encoder;
        std::thread m_stream_thread;
        std::atomic<bool> stream_shutdown;

        struct stream_entry *m_stream_context;

        //! Reads the entire Payload form message and copies it to
        //! a uint8_t array
        uint8_t * payload_to_frame (videostream::FromFeed& message);

        bool alloc_encoder (int width, int height, enum AVPixelFormat fmt);

        void handle_incoming_message (videostream::FromFeed& message);

        void read_incoming_message (videostream::FromFeed& message);

        bool on_stream_hung_up ();

        void handle_traffic ();

        void run_threaded ()
        {
            m_stream_thread = std::thread (&NetStream::handle_traffic, this);
        }

        struct stream_entry *wait_for_stream_context ();
    };
}

#endif //JVH_STREAM_ENCODE_TCP_H
