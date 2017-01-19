#include <iostream>
#include <fstream>

#include "client.h"
#include "server.h"

using namespace jvh;

Client::Client (Server *server) :
    m_server (server)
{

}

Client::~Client ()
{

}

bool
Client::on_hung_up (Glib::IOCondition)
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
        auto& entry = m_server->stream_entries().at(entry_string);

        std::ifstream file_buffer(entry->se_filepath, std::ifstream::binary);
        std::cout << "opening file: " << entry->se_filepath << std::endl;
        if (file_buffer.is_open())
        {
            while (true)
            {
                // No more to read
                if (file_buffer.eof())
                {
                    break;
                }

                char buffer[50000];
                file_buffer.read(buffer, 50000);

                if (file_buffer.fail())
                {
                    std::cerr << "fail on file_buffer" << std::endl;
                    break;
                }
                if (file_buffer.gcount () == 0)
                {
                    // Nothing more to read. Break out
                    break;
                }

                videostream::ToClient msg;
                auto& payload (*msg.mutable_payload());
                payload.add_payload(buffer, file_buffer.gcount ());

                //std::cout << "Sending payload..: " << msg.ShortDebugString() << std::endl;
                send_outgoing_message (msg);
            }
        }
    }
    default:
        // Unknown message - do nothing
        break;
    }
}

Client::handle_traffic ()
{
    std::cerr << "Started new client thread" << std::endl;

    while (!teardown_tread)
    {
        // listens to incoming
        // connections and are handled
        // within the function
        try
        {
            traffic_listen ()
        }
        catch (exception& e)
        {
            std::cerr << e.what () << std::endl;
        }
    }
}




