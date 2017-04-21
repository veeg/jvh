#include <stdint.h>
#include <iostream>

namespace jvh
{
    class Server;
    //! Class to hide
    class Videostream
    {
    public:
        Videostream ();
        ~Videostream ();

        //! \brief Starts websocket to which clients
        //! connect
        void serve_websocket (const uint32_t port);

        //! \brief Starts tcp to which client connect
        void serve_tcpsocket (const uint32_t port);

        //! \brief Starts listener for incoming
        //! video feeds
        void setup_streamsocket (const uint32_t port);
        void register_stream_entry_file (std::string name, std::string filepath, std::string format,
                                uint32_t width, uint32_t height);

        void set_videodirectory (const char *dir);

        void run ();

        void close ();

    private:
        Server *impl;
    };
}
