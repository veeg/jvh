#include <videostream/videostream.h>
#include <memory>


int main (int argc, char **argv)
{
    auto vs = std::make_unique<jvh::Videostream> ();

    vs->serve_websocket (8001);

    vs->run ();
}
