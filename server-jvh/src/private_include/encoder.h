#ifndef JVH_ENCODER_H
#define JVH_ENCODER_H

#include <functional>
#include <thread>
#include "mqueue.h"
#include "av_util.h"
#include <atomic>

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
        Encoder (uint32_t frame_width, uint32_t frame_height, enum AVPixelFormat fmt,  AVCodecID output_codec);
        ~Encoder ();

        //! \brief sends a frame to be encoded
        //!
        //! XXX: must be of size returned from
        //! get_frame_buffer_size
        void send_frame (uint8_t *data);

        //! \brief signal end-of-stream to the encoder
        //!
        //! encoder will drain input and quit
        //!
        void drain ();

        //! \brief kicks off the encoder thread which waits for incoming frames
        //!
        //! \param enqueue_packet function to be called when a frame is encoded
        //!
        void start (std::function<void(AVPacket *pkt)> enqueue_packet);

        uint32_t get_frame_buffer_size ();

    protected:
        AVCodecContext *m_codec_ctx;

        uint32_t m_frame_size;

        //! \brief encodes frames from m_frame_queue and
        //! sends the encoded frame to the callback "enqueue_packet"
        void encode (std::function<void(AVPacket *pkt)> enqueue_packet);

        //! Converts raw frames into AVFrame structures
        //! according to the parameters in m_codec_ctx
        AVFrame* populate_frame (AVFrame *frame, uint8_t *data);

        //! Signals the encoder to drain
        //! the encode buffers
        //!
        //! set if input has ended
        std::atomic<bool> m_drain;
        // holds ready-to-be encoded data
        MQueue<uint8_t*> m_frame_queue;

        std::thread m_encode_thread;
    };
}

#endif
