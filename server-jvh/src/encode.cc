#include <libavcodec/avcodec.h>

using namespace jvh;

Encoder::Encoder (int codec_id, int stream_fd)
{
    AVCodec *codec;
    AVCodecContext *c = NULL;
    AVFrame *frame;
    AVPacket pkt;

    codec = avcodec_find_encoder (m_codec_id);
    if (codec == NULL)
    {
        throw std::runtime_error("Codec not found");
    }

    c = avcodec_alloc_context3 (codec);
    if (c == NULL)
    {
        throw std::runtime_error ("Could not allocate context");
    }

    // Change this please
    c->bit_rate = 400000;

    c->width = m_width;
    c->height = m_height;

    c->gop_size = 10;
    c->max_b_frames = 1;
    c->pix_fmt = m_pixel_fmt;

    if (avcodec_open2 (c, codec, NULL) < 0 )
    {
        //maybe close code on error
        throw std::runtime_error ("Could not open codec.");
    }

    frame = av_frame_alloc ()
    if (frame == NULL)
    {
        // maybe close codec on error?
        throw std::runtime_error ("Could not allocate video frame");
    }

    frame->format = c->pix_width;
    frame->width = c->width;
    frame->height = c->height;

    // size of frame int bytes
    m_frame_size = (c->height * c->width);
}

Encoder::~Encoder ()
{

}

PCQueue<uint32_t[]>
Encoder::stream_subscribe ()
{
      
}

Encoder::encode_frame (AVFrame *frame)
{
    AVPacket *pkt = av_packet_alloc ();
    int ret;

    ret = avcodec_send_frame (m_codec_context, frame);
    if (ret < 0)
        goto out;

    while (!ret)
    {
        ret = avcodec_recieve_packet (m_codec_context, pkt);
        enqueue_outgoing (pkt)
    }
}

Encoder::enqueue_frame (const uint8_t *data)
{
    AVFrame *frame = avcodec_alloc_frame ();

    frame->width = m_frame_width;
    frame->height = m_frame_height;
    frame->format = m_frame_format;

    avpicture_fill ((AVPicture*)frame, data, m_pix_fmt, m_frame_width, m_frame_height);

    encode_frame(frame);
}

void
Encoder::enqueue_outgoing (AVPacket *pkt)
{
   for (auto iterator = m_stream_out.begin (); iterator != m_stream_out.end ();
        ++iterator)
    {
        *iterator.enqueue (pkt);
    }
}

