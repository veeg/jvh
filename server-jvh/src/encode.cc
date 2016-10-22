#include <libavcodec/avcodec.h>

using namespace jvh;


Encoder::Encoder ()
{

}

Encoder::~Encoder ()
{

}

uint8_t *
Encoder::encode_frame (uint8_t *frame)
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


}
