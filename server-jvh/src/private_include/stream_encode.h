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
        StreamEncoded ();
        ~StreamEncoded ();

        //! \brief starts stream encode to run in a separate thread
        virtual void start (struct stream_entry *entry);

        void read_from_video_source (const char *filepath, Encoder *encoder);
        virtual std::shared_ptr<StreamQueue> subscribe (void *client);

        virtual void unsubscribe (void *client);

        //virtual void set_stream_input ();
    private:
        void shutdown_stream ();

        //! used to lock the queue map
        std::mutex m_qlock;

        //! \brief populates all outgoing queues with pkt
        void enqueue_outgoing (AVPacket *pkt);

        //! Such that we know how large each
        //! chunk needs to be
        uint32_t m_frame_size;

        std::map<void*, std::shared_ptr<StreamQueue>> m_outgoing_streams;
    };
}

#endif // JVH_STREAM_ENCODE_H
