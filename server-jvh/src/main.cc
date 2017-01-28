#include "server.h"

int  main (int argc, char **argv)
{
    jvh::Server server;

    server.serve_websocket (8001);

    //server.register_stream_entry_file ("parrot", "/home/veeg/video/parrot.webm", "webm", "vp8");

    server.stream_file_encode ("lol", "/home/helge/Documents/jvh/claire_qcif.yuv", "176x144", "yuv");
    server.start_listening ();
}
