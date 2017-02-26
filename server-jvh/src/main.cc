#include "server.h"
#include "videostream.pb.h"
#include <thread>
#include "client_tcp.h"
#include "unistd.h"
#include "sys/wait.h"
#include <iostream>
#include <fstream>

void start_client_stream (int port, std::atomic<bool>& wait_for_server);

int  main (int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: server-jvh port" << std::endl;
        exit (0);
    }

    jvh::Server *server = new jvh::Server ();
    auto port = std::stoi (argv[1]);
    pid_t pid;
    std::atomic<bool> wait_for_server (true);

    server->serve_socket_tcp (port);

    server->stream_file_encode ("lol", "/home/helge/Documents/jvh/test2",
                                "yuv", 800, 600);

    std::thread client (start_client_stream, port, std::ref (wait_for_server));

    wait_for_server = false;

    server->start_listening ();
}

int
setup_connection (const int port)
{
    struct sockaddr_in sockaddr;
    int sock;
    int ret;

    sock = socket (AF_INET, SOCK_STREAM, 0);
    sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons (port);

    ret = connect(sock , (struct sockaddr *)&sockaddr , sizeof(sockaddr));
    return ret < 0 ? ret : sock;
}

void
start_client_stream (int port, std::atomic<bool>& wait_for_server)
{
    videostream::FromClient from_client;
    videostream::ToClient to_client;
    std::string message;
    int sock;
    jvh::ProtoTCP *pbcomm;

    while (wait_for_server.load ()) {}

    sock = setup_connection (port);
    if (sock < 0)
    {
        std::cerr << strerror (errno) << std::endl;
        return;
    }
    std::cerr << "connected" << std::endl;

    pbcomm = new jvh::ProtoTCP (sock);

    from_client.set_request_stream_entries (true);

    std::cerr << "sending message" << std::endl;
    if (pbcomm->send (from_client) == false)
    {
        std::cerr << strerror (errno) << std::endl;
        return;
    }

    if (pbcomm->receive (to_client) == false)
    {
        return;
    }
    std::cerr << "got message" << std::endl;

    std::cerr <<  "CONTENT: " << to_client.ShortDebugString () << std::endl;
    from_client.Clear ();

    switch (to_client.msg_case ())
    {
    case videostream::ToClient::kStreamEntries:
        for (auto& entry : to_client.stream_entries().entries ())
        {
            std::cerr << entry.name() << std::endl;
        }
        break;
    default:
        std::cerr << "Not what i wanted!!" << std::endl;
    }
    from_client.set_select_stream_entry ("lol");
    message.clear ();

    if (pbcomm->send (from_client) == false)
    {
        std::cerr << strerror (errno) << std::endl;
        return;
    }
    to_client.Clear ();


    std::ofstream output;
    output.open ("test.mpg");
    std::cerr << "Ready to recieve" << std::endl;
    int received = 0;
    while (pbcomm->receive (to_client))
    {
        if (to_client.has_payload ())
        {
            auto& payload = (*to_client.mutable_payload ());
            for (auto &data: payload.payload ())
            {
                output << data;
                output.flush ();
                std::cerr << "Got packet" << std::endl;
            }
        }
        to_client.Clear ();
    }
    output.close ();
    close (sock);
}

