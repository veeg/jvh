#ifndef JVH_STREAM_ENCODE_H
#define JVH_STREAM_ENCODE_H

#include "stream.h"
#include <list>
#include <queue>
#include "mqueue.h"
#include <memory>
#include <thread>
#include "encoder.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace jvh
{
    //! Childclass which encodes raw video
    class StreamEncoded : public Stream
    {
    public:
        //! \brief
        StreamEncoded (struct stream_entry *entry);
        ~StreamEncoded ();

        //! \brief subscribe to stream
        //!
        //!
        virtual std::shared_ptr<StreamQueue> subscribe (void *client);

        virtual void unsubscribe (void *client);

        //virtual void set_stream_input ();
    private:
        void shutdown_stream ();


        std::thread start_unique_feed (std::shared_ptr<StreamQueue> outgoing);

        //! \brief populates all outgoing queues with pkt
        //!
        //! Invoked by encoder to enqueue processed frames
        //! \param outgoing is bound to the callback
        //! sent to the encoder and will be unique per client
        //!
        void enqueue_outgoing (AVPacket *pkt, std::shared_ptr<StreamQueue> outgoing);

        void read_from_video_source (const char *filepath, Encoder *encoder);
        virtual bool is_active ();

        virtual void start (struct stream_entry *entry) {}

        struct stream_entry *m_stream_entry;

        //! Such that we know how large each
        //! chunk needs to be
        //! used to lock the queue map
        std::mutex m_qlock;
        uint32_t m_frame_size;
        std::thread m_read_thread;
        std::vector<std::thread> m_read_threads;
        std::map<void*, std::shared_ptr<StreamQueue>> m_outgoing_streams;
    };
}

#endif // JVH_STREAM_ENCODE_H
