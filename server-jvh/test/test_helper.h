#ifndef JVH_VIDEOSTREAM_TEST_H
#define JVH_VIDEOSTREAM_TEST_H

class VideostreamTest public testing::Test
{
protected:
    void SetUp ()
    {
        server.stream_file_encode ("test",
        "/home/helge/Documents/jvh/vid/test2", 800, 600);
        server.serve_socket_tcp (8000);

        m_client = new ClientTCP (8000);

        m_video_file.open ("videotest");
    }
    void TearDown ()
    {
        if (m_server)
        {
            delete m_server;
        }
        if (m_client)
        {
            delete m_client;
        }

        m_video_file.close ();
    }
    void send_message (videostream::FromClient& message)
    {

        m_client->send_outgoing_message (message);
    }

    void receive_message (videostream::ToClient& message)
    {
        m_client->receive_message (message);
    }

    void

public:
    jvh::Server *m_server;
    jvh::ClientTCP *m_client;

    videostream::ToClient ToClient;
    videostream::FromClient ToClient;

    // used to store video
    // from server
    ofstream m_video_file;
};

#endif
