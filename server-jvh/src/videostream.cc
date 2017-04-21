#include <videostream/videostream.h>

#include "server.h"

using namespace jvh;

Videostream::Videostream ()
{
    impl = new Server ();
}

Videostream::~Videostream ()
{
    delete impl;
}

void
Videostream::serve_websocket (const uint32_t port)
{
    impl->serve_websocket (port);
}

void
Videostream::serve_tcpsocket (const uint32_t port)
{
    impl->serve_socket_tcp (port);
}

void
Videostream::set_videodirectory (const char *dir)
{}

void
Videostream::run ()
{
    impl->start_listening ();
}

void
Videostream::close ()
{
    impl->close_sockets ();
}

void
Videostream::setup_streamsocket (const uint32_t port)
{
    impl->serve_streams_incoming (port);
}

