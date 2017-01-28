#include <iostream>
#include <fstream>
#include "client.h"
#include "server.h"
#include <videostream.pb.h>

using namespace jvh;

Client::Client (Server *server) :
    m_server (server)
{

}

bool
Client::on_hung_up ()
{
    std::cerr << "Disconnecting client trafic" << std::endl;
    m_signal_disconnected (this);
    return false;
}

void
Client::handle_incoming_message (const videostream::FromClient& message)
{
    std::cerr << "CONTENT: " << message.ShortDebugString () << std::endl;

    switch (message.msg_case ())
    {
    case videostream::FromClient::kRequestStreamEntries:
    {
        std::cerr << "Got RequestStreamEntries" << std::endl;

        videostream::ToClient msg;
        auto& entries (*msg.mutable_stream_entries());

        // XXX: Get m_stream_entries from the server.
        for (const auto &e : m_server->stream_entries ())
        {
            auto entry = entries.add_entries ();
            entry->set_name (e.second->se_name);
            entry->set_format (e.second->se_format);
            entry->set_codec (e.second->se_codec);
        }

        send_outgoing_message (msg);
        break;
    }
    case videostream::FromClient::kSelectStreamEntry:
    {
        // TEST: Just read the file inline in chunks and send chunks one after another
        // until the while file is read. That way, we will effectively block the server from
        // handling anything until it either throws an exception or finished serving it all

        // Lookup stream entry
        std::string entry_string = message.select_stream_entry ();
        std::cout << "client requested stream entry: " << entry_string << std::endl;
        m_stream = m_server->get_stream (entry_string);
        m_stream->subscribe (this);
    }
    default:
        // Unknown message - do nothing
        break;
    }
}

void
Client::handle_traffic ()
{
    std::cerr << "Started new client thread" << std::endl;

    while (true)
    {
        if (incoming_timeout (100))
        {
            read_incoming_message ();
        }

        if (m_stream && !m_stream_queue->empty ())
        {
            auto frame = m_stream_queue->front ();
            m_stream_queue->pop ();

            send_outgoing_message (*frame);
        }
        // if stream is shut down and queue is empty
        else if (m_stream && m_stream_queue->empty ())
        {
            if (!m_stream->is_active ())
            {
                m_stream->unsubscribe (this);
            }
        }
    }
}


