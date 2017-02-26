#include "encoder.h"
#include <assert.h>
#include <iostream>

#include <fstream>

using namespace jvh;

Encoder::~Encoder ()
{
    m_encode_thread.join ();
    avcodec_free_context (&m_codec_ctx);
}

// TODO: pixel format of input data
Encoder::Encoder (uint32_t frame_width, uint32_t frame_height,
                  enum AVPixelFormat fmt, AVCodecID output_codec)
{
    AVCodec *codec;

    avcodec_register_all ();
    av_register_all ();

    codec = avcodec_find_encoder (output_codec);
    if (codec == NULL)
    {
        throw std::runtime_error("Codec not found");
    }

    m_codec_ctx = avcodec_alloc_context3 (codec);
    if (m_codec_ctx == NULL)
    {
        throw std::runtime_error ("Could not allocate context");
    }

    m_codec_ctx->width = frame_width;
    m_codec_ctx->height = frame_height;
    m_codec_ctx->bit_rate = 40000;
    m_codec_ctx->gop_size = 10;
    m_codec_ctx->time_base = (AVRational){1,25};
    m_codec_ctx->max_b_frames = 1;
    m_codec_ctx->time_base.den = 25;
    m_codec_ctx->time_base.num = 1;
    m_codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    m_codec_ctx->pix_fmt = fmt;

    if (avcodec_open2 (m_codec_ctx, codec, NULL) < 0 )
    {
        //maybe close code on error
        throw std::runtime_error ("Could not open codec.");
    }

    // Determine how many bytes a frame is composed of
    m_frame_size = av_image_get_buffer_size (m_codec_ctx->pix_fmt, m_codec_ctx->width,
                                             m_codec_ctx->height, 1);
    if (m_frame_size < 0)
    {
        throw std::runtime_error ( av_error_desc (m_frame_size));
    }

    m_drain.store (false);
}

uint32_t
Encoder::get_frame_buffer_size ()
{
    return m_frame_size;
}

void
Encoder::send_frame (uint8_t *data)
{
    m_frame_queue.enqueue (data);
}

void
Encoder::start (std::function<void(AVPacket *pkt)> enqueue_packet)
{
    m_encode_thread = std::thread (&Encoder::encode, this, enqueue_packet);
}

AVFrame*
Encoder::populate_frame (AVFrame *frame, uint8_t *data)
{
    int ret;

    // Populate the frame structure according to its format
    ret = av_image_fill_arrays (frame->data, frame->linesize, (const uint8_t *)data,
                                       m_codec_ctx->pix_fmt, m_codec_ctx->width,
                                       m_codec_ctx->height, 1);

    // ret should be equal to
    // the image frame size
    assert (ret == m_frame_size);

    return frame;
}

void
Encoder::drain (void)
{
    m_drain = true;
}

void
Encoder::encode (std::function<void(AVPacket *pkt)> enqueue_packet)
{
    std::cerr << "In stream encode"
              << std::endl;

    AVFrame *frame = av_frame_alloc ();
    if (frame == NULL)
    {
        throw std::runtime_error ("Could not allocate frame");
    }

    AVPacket pkt;
    int ret, still_encoding;
    uint64_t pts = 0;

    frame->format = m_codec_ctx->pix_fmt;
    frame->width = m_codec_ctx->width;
    frame->height = m_codec_ctx->height;

    // run loop until the queue is empty and the encoder
    // has been signaled for drainage
    while (m_drain.load () == false || m_frame_queue.size () != 0)
    {
        auto data = m_frame_queue.dequeue ();
        assert (data != NULL);

        populate_frame (frame, data);

        // Dictates at which
        // time the frame is shown
        frame->pts = pts++;

        ret = avcodec_send_frame (m_codec_ctx, frame);
        if (ret < 0)
        {
            // XXX: Currently just loggin the erro
            std::cerr << av_error_desc (ret)
                      << std::endl;
        }

        pkt.size = 0;
        pkt.data = NULL;

        // Loop until packet is encoded
        still_encoding = 0;
        while (!still_encoding)
        {
            still_encoding = avcodec_receive_packet (m_codec_ctx, &pkt);
            if (!still_encoding)
            {
                assert (still_encoding == 0);
                enqueue_packet (&pkt);
                av_packet_unref (&pkt);
            }
        }
    }
    std::cerr << "Exiting encode: " << m_drain.load () << std::endl;
    av_frame_free (&frame);
}

