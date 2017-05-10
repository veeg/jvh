#include "stream_capture.h"
#include <videostream.pb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace capture;

StreamComm::StreamComm ()
{
}

void
StreamComm::connect_to_server (uint32_t port, const char *ip)
{
    int sockfd;
    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket (AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        throw std::runtime_error ("failed to open socket.");
    }

    bzero ((char*) &serv_addr, sizeof (serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    // XXX: Change this to enable other than localhost connections
    serv_addr.sin_addr.s_addr = inet_addr (ip);

    if (connect (sockfd, (struct sockaddr*)&serv_addr, sizeof (sockaddr)) < 0)
    {
        throw std::runtime_error ("Could not connect to socket");
    }

    m_protoTCP = new jvh::ProtoTCP (sockfd);
}

StreamComm::~StreamComm ()
{
    delete m_protoTCP;
}

void
StreamComm::send_capture_context (videostream::StreamContext& info)
{
    videostream::FromFeed fromfeed;

    auto& stream_ctx (*fromfeed.mutable_stream_context ());

    stream_ctx = info;

    m_protoTCP->send (fromfeed);
}

void
StreamComm::send_frame (char *frame, uint32_t size)
{
    videostream::FromFeed message; //= new videostream::FromFeed ();

    auto& payload (*message.mutable_payload ());

    payload.add_payload (frame, size);

    m_protoTCP->send (message);
}
/*
StreamComm::handle_traffic ()
{
    while (m_shutdown.load () == false)
    {

    }
}
*/
