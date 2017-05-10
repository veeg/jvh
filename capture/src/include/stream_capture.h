#include "prototcp.h"
#include <videostream.pb.h>

namespace capture
{
    class StreamComm
    {
    public:
        StreamComm ();
        ~StreamComm ();
        void connect_to_server (uint32_t port, const char *ip);
        void send_frame (char *frame, uint32_t size);
        void send_capture_context (videostream::StreamContext& info);
    private:
        jvh::ProtoTCP *m_protoTCP;
    };
}
