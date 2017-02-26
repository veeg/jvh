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

StreamEncoded::StreamEncoded (struct stream_entry *entry)
{
    // XXX: Unfortunate to share that pointer
    m_stream_entry = entry;
    m_stream_shutdown.store (true);

}

StreamEncoded::~StreamEncoded ()
{
    for (auto& thread : m_read_threads)
    {
        thread.join ();
    }
}

void
StreamEncoded::read_from_video_source (const char *filepath, Encoder *encoder)
{
    std::cerr << "reading file" << std::endl;

    std::ifstream vidfile (filepath, std::ifstream::binary);
    uint8_t *chunk;

    if (!vidfile.good ())
    {
        std::cerr << "File: " << filepath
                  << " does not exist" << std::endl;
        goto out;
    }

    // Determine, in number of bytes, how
    // large one frame is
    m_frame_size = encoder->get_frame_buffer_size ();

    // XXX: implement shutdown
    // on client unregister
    while (!vidfile.eof ())
    {
        chunk = new uint8_t [m_frame_size];
        vidfile.read ((char*) chunk, m_frame_size);

        if (vidfile.gcount () != m_frame_size)
        {
            // guess we're on the last chunk
            break;
        }

        if (vidfile.fail ())
        {
            std::cerr << "Failed to read file" << std::endl;
            break;
        }
        if (vidfile.gcount () == 0)
        {
            break;
        }

        encoder->send_frame (chunk);
    }

out:
    encoder->drain ();
}

std::thread
StreamEncoded::start_unique_feed (std::shared_ptr<StreamQueue> outgoing)
{
    Encoder *encoder = new Encoder (m_stream_entry->se_width,
                                    m_stream_entry->se_height,
                                    AV_PIX_FMT_YUV420P,
                                    AV_CODEC_ID_MPEG1VIDEO);

    auto fp = std::bind (&StreamEncoded::enqueue_outgoing,
                         this, std::placeholders::_1, outgoing);

    encoder->start (fp);

    return std::thread(&StreamEncoded::read_from_video_source,
                        this, m_stream_entry->se_filepath.c_str (), encoder);
}

// Currently Unused
void
StreamEncoded::shutdown_stream ()
{
    std::cerr << "shutting down stream"
              << std::endl;
}

// Callback function from the encoder
void
StreamEncoded::enqueue_outgoing (AVPacket *pkt, std::shared_ptr<StreamQueue> outgoing)
{
   std::shared_ptr<videostream::ToClient> msg(new videostream::ToClient);

   std::cerr << "enqueueing " << std::endl;

   auto& payload (*msg->mutable_payload ());
   payload.add_payload (pkt->data, pkt->size);

   outgoing->push (msg);
}

std::shared_ptr<StreamQueue>
StreamEncoded::subscribe (void *client)
{
    std::unique_lock<std::mutex> lock (m_qlock);
    std::cerr << "In Subscribe" << std::endl;

    auto queue = std::make_shared<StreamQueue> ();

    m_read_threads.push_back (start_unique_feed (queue));

    m_outgoing_streams[client] = queue;

    return queue;
}

void
StreamEncoded::unsubscribe (void *client)
{
    std::unique_lock<std::mutex> lock (m_qlock);

    m_outgoing_streams.erase (client);
}
