#include "stream_encode.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <stdint.h>
#include "mqueue.h"


extern "C" {
    #include <libavformat/avformat.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/frame.h>
}

using namespace jvh;

StreamEncoded::StreamEncoded ()
{

}


StreamEncoded::~StreamEncoded ()
{
}

void
StreamEncoded::read_from_video_source (const char *filepath, Encoder *encoder)
{
    std::ifstream infile (filepath);
    uint8_t *chunk;

    while (!infile.eof ())
    {
        chunk = new uint8_t[m_frame_size];
        infile >> chunk;
        encoder->send_frame (chunk);
    }

    delete encoder;
    m_stream_shutdown = true;
}

void
StreamEncoded::start (struct stream_entry *entry)
{
    Encoder *encoder = new Encoder (entry->se_height, entry->se_width, AV_CODEC_ID_VP8);

    std::thread read_thread (
            &StreamEncoded::read_from_video_source, this, entry->se_filepath.c_str (), encoder);
}

void
StreamEncoded::shutdown_stream ()
{
    std::cerr << "shutting down stream"
              << std::endl;
}

void
StreamEncoded::enqueue_outgoing (AVPacket *pkt)
{
   std::shared_ptr<videostream::ToClient> msg(new videostream::ToClient);

   auto& payload (*msg->mutable_payload ());
   payload.add_payload (pkt->data, pkt->size);

   // Lock the map to ensure no race conditions
   // if a client concurrently calls subscribe or unsubscribe
   std::unique_lock<std::mutex> lock (m_qlock);

   for (auto iterator = m_outgoing_streams.begin (); iterator != m_outgoing_streams.end ();
        ++iterator)
    {
        (*iterator).second->push (msg);
    }
}

std::shared_ptr<StreamQueue>
StreamEncoded::subscribe (void *client)
{
    std::unique_lock<std::mutex> lock (m_qlock);

    auto queue = std::make_shared<StreamQueue> ();

    m_outgoing_streams[client] = queue;

    return queue;
}

void
StreamEncoded::unsubscribe (void *client)
{
    std::unique_lock<std::mutex> lock (m_qlock);

    m_outgoing_streams.erase (client);
}
