#ifndef JVH_STREAM_INPUT_H
#define JVH_STREAM_INPUT_H

#include <atomic>
#include "videostream.pb.h"
#include <memory>
#include "mqueue.h"

namespace jvh
{
    enum stream_type {
        STREAM_TYPE_FILE = 0,
        STREAM_TYPE_FILE_ENCODE,
        STREAM_TYPE_ENCODE_FEED,
    };

    //! Structure holds a single entry that is streamable to clients
    //! @param se_strem has functionality to dequeue frames
    struct stream_entry {
        std::string se_name;
        //! If the streaming entry is a file, this filepath is the location of said file
        std::string se_format;
        std::string se_codec;
        std::string se_filepath;
        std::string se_framesize;
        uint32_t    se_height;
        uint32_t    se_width;
        enum stream_type se_type;
    };

    typedef std::queue<std::shared_ptr<videostream::ToClient>> StreamQueue;

    class Stream
    {
    public:
        Stream () {}
        virtual ~Stream () {}

        //! \brief Subscribe to the given stream
        //!
        //! dequeue messages until stream is no longer
        //! active and StreamQueue is empty
        virtual std::shared_ptr<StreamQueue> subscribe (void *client) = 0;

        virtual void unsubscribe (void *client) = 0;

        virtual bool is_active ();

    protected:
        std::atomic<bool> m_stream_shutdown;

    };
}

#endif //JVH_STREAM_INPUT_H
