#include "encoder.h"
#include <iostream>
#include "av_util.h"

using namespace jvh;

Encoder::~Encoder ()
{
    m_encode_thread.join ();
    avcodec_free_context (&m_codec_ctx);
}

// TODO: pixel format of input data
Encoder::Encoder (uint32_t frame_width, uint32_t frame_height,
                  AVCodecID output_codec)
{
    AVCodec *codec;

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

    // Change this please
    m_codec_ctx->bit_rate = 400000;

    m_codec_ctx->width = frame_width;
    m_codec_ctx->height = frame_height;

    m_codec_ctx->gop_size = 10;
    m_codec_ctx->max_b_frames = 1;
    m_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    if (avcodec_open2 (m_codec_ctx, codec, NULL) < 0 )
    {
        //maybe close code on error
        throw std::runtime_error ("Could not open codec.");
    }

    // Determine how many bytes a frame is composed of
    m_frame_size = av_image_get_buffer_size (m_codec_ctx->pix_fmt,
                                       m_codec_ctx->width, m_codec_ctx->height, 1);
    if (m_frame_size < 0)
    {
        throw std::runtime_error ( av_error_desc (m_frame_size));
    }
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
Encoder::populate_frame (uint8_t *data)
{
    AVFrame *frame = av_frame_alloc ();

    frame->width = m_codec_ctx->width;
    frame->height = m_codec_ctx->height;;
    frame->format = m_codec_ctx->pix_fmt;

    // Populate the frame structure according to its format
    av_image_fill_arrays (frame->data, frame->linesize, (const uint8_t *)data, m_codec_ctx->pix_fmt,
                          m_codec_ctx->width, m_codec_ctx->height, 1);
    return frame;
}

void
Encoder::encode (std::function<void(AVPacket *pkt)> enqueue_packet)
{
    std::cerr << "In stream encode"
              << std::endl;

    AVPacket *pkt;
    AVFrame *frame;
    int ret, still_encoding;

    while (true)
    {
        auto raw_frame = m_frame_queue.dequeue ();
        frame = populate_frame (raw_frame);

        delete[] raw_frame;

        ret = avcodec_send_frame (m_codec_ctx, frame);
        if (ret < 0)
            goto err;

        pkt = av_packet_alloc ();
        // Loop until packet is encoded
        while (!still_encoding)
        {
            still_encoding = avcodec_receive_packet (m_codec_ctx, pkt);
            if (!still_encoding)
            {
                enqueue_packet (pkt);
                av_frame_free (&frame);
            }
        }
        av_packet_unref (pkt);
    }
err:
    std::cerr << "stream encode exit"
              << std::endl;
}

