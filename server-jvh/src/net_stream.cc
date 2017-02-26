#include "net_stream.h"
#include "av_util.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavutil/imgutils.h>
    #include <libavformat/avformat.h>
}

using namespace jvh;

NetStream::NetStream (int socket)
{
    m_pbcomm = new ProtoTCP (socket);
}


NetStream::~NetStream ()
{
    if (m_pbcomm)
    {
        delete m_pbcomm;
    }
    if (m_encoder)
    {
        delete m_encoder;
    }
}

bool
NetStream::on_stream_hung_up ()
{
    m_signal_disconnected (this);
}

bool
NetStream::alloc_encoder (int width, int height, enum AVPixelFormat fmt)
{
    try
    {
        m_encoder = new Encoder (width, height, fmt, AV_CODEC_ID_MPEG1VIDEO);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what () << std::endl;
        // XXX: Maybe let the other end know
        // they fucked up
        return false;
    }
    return true;
}

uint8_t *
NetStream::payload_to_frame (videostream::FromFeed& message)
{
    auto frame_buffer = new uint8_t[m_encoder->get_frame_buffer_size ()];
    auto& payload  (*message.mutable_payload ());
    int current_lenght = 0;

    for (auto&data: payload.payload ())
    {
        memcpy (&frame_buffer[current_lenght], data.c_str (), data.length ());
    }
}

void
NetStream::handle_incoming_message (videostream::FromFeed& message)
{
    videostream::ToFeed response;

    switch (message.msg_case ())
    {
    // XXX: should only be allowed once
    case videostream::FromFeed::kStreamContext:
    {
        auto& stream_context = message.stream_context ();

        auto height = stream_context.frame_height ();
        auto width = stream_context.frame_width ();
        auto v4l2_pix_fmt = stream_context.pixel_format ();
        auto frame_buffer_size = stream_context.frame_buffer_size ();

        // Convert the v4l2_pix_fmt enum to the corresponding libavcodec pix_fmt
        auto libav_pix_fmt = ff_fmt_v4l2ff (v4l2_pix_fmt, AV_CODEC_ID_RAWVIDEO);
        if (libav_pix_fmt == AV_PIX_FMT_NONE)
        {
            response.set_context_error_string ("Pixel format unknown");
            break;
        }

        // Try to new-up an encoder with the given context
        // if either codec or format is not supported - notify the feed
        if (alloc_encoder(width, height, libav_pix_fmt) == false)
        {
            response.set_context_error_string ("Context error");
            break;
        }

        // The encoder calculates a frame size in bytes according
        // to the parameters given. If the sizes do not match,
        // the feed has provided an incorrect context
        if (frame_buffer_size != m_encoder->get_frame_buffer_size ())
        {
            response.set_frame_size_mismatched (true);
            break;
        }

        auto se = new stream_entry;
        se->se_height = height;
        se->se_width = width;


        response.set_ready_to_receive_data (true);

        m_pbcomm->send (response);
        break;
    }
    case videostream::FromFeed::kPayload:
    {
        // Cannot recieve payload if no encoder
        // nor stream_context is set
        if (m_stream_context == NULL ||
            m_encoder == NULL)
        {
            response.set_context_error_string ("Cannot recieve payload");
            m_pbcomm->send (response);
            break;
        }

        m_encoder->send_frame (payload_to_frame (message));
        break;
    }
    default:
        std::cerr << "Which message is this? " << std::endl;
        break;
    }
}

struct stream_entry*
NetStream::wait_for_stream_context ()
{

}

void
NetStream::read_incoming_message (videostream::FromFeed& message)
{
    if (m_pbcomm->receive (message) == false)
    {
        on_stream_hung_up ();
    }
}

void
NetStream::handle_traffic ()
{
    videostream::FromFeed message;
    while (stream_shutdown.load () == false)
    {
        read_incoming_message (message);

        handle_incoming_message (message);

        message.Clear ();
    }
}

std::shared_ptr<StreamQueue>
NetStream::subscribe (void *client)
{


}

void
NetStream::unsubscribe (void *client)
{

}
