#ifndef JVH_ENCODER_H
#define JVH_ENCODER_H

#include <functional>
#include <thread>
#include "mqueue.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/frame.h>
}

namespace jvh
{
    class Encoder
    {
    public:
        Encoder (uint32_t frame_width, uint32_t frame_height, AVCodecID output_codec);
        ~Encoder ();

        //! sends a frame to be encoded
        void send_frame (uint8_t *data);

        //! \brief kicks off the encoder thread which waits for incoming frames
        //!
        //! \param enqueue_packet function to be called when a frame is encoded
        //!
        void start (std::function<void(AVPacket *pkt)> enqueue_packet);

        uint32_t get_frame_buffer_size ();

    protected:
        AVCodecContext *m_codec_ctx;

        uint32_t m_frame_size;

        void encode (std::function<void(AVPacket *pkt)> enqueue_packet);

        AVFrame* populate_frame (uint8_t *data);

        MQueue<uint8_t*> m_frame_queue;

        std::thread m_encode_thread;
    };
}

#endif
