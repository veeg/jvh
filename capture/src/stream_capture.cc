#include "stream_capture.h"
#include "prototcp.h"
#include <videostream.pb.h>

using namspace capture;

StreamComm::StreamComm (uint32_t port)
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
    serv_addr.sin_port = htons(feed_port);
    // XXX: Change this to enable other than localhost connections
    serv_addr.sin_addr.s_addr = inet_addr ("127.0.0.1");

    if (connect (sockfd, (struct sockaddr)&sockaddr, sizeof (sockaddr) < 0)
    {
        throw std::runtime_error ("Could not connect to socket");
    }

    m_protoTCP = new ProtoTCP(socket);
}

StreamComm::~StreamComm ()
{
    delete m_protoTCP;
}

StreamComm::send_capture_context ()
{


}

StreamComm::send_frame (uint8_t *frame, uint32_t size)
{
    videostream::FromFeed message = new videostream::ToClient();

    auto& payload (*msg->mutable_payload ());

    payload.add_payload (frame, size);

}

StreamComm::handle_traffic ()
{
    while (m_shutdown.load () == false)
    {

    }
}
