#ifndef JVH_SERVER_H
#define JVH_SERVER_H

#include <nopoll.h>
#include <list>
#include <memory>
#include <map>
#include <glibmm/main.h>
#include "videostream.pb.h"
#include "client.h"

namespace jvh
{
    // XXX ??? Dont do this
    typedef int client;

    //! Structure holds a single entry that is streamable to clients
    struct stream_entry {
        std::string se_name;
        //! If the streaming entry is a file, this filepath is the location of said file
        std::string se_filepath;
        std::string se_format;
        std::string se_codec;
    };

    class Server
    {
    public:
        Server ();
        ~Server ();

        void serve_websocket (const uint32_t client_port);
        void serve_feed_socket (const uint32_t feed_port);
        void close_sockets ();
        void start_listening ();
        // Get list of client identifiers and then use the id to send the respective client
        // a message
        std::list<client> get_clientids();
        //! Send a protobuff message to the client on id id
        int send_message(client id, videostream::ToClient m);

        void register_stream_entry_file (std::string name, std::string filepath,
                                         std::string format, std::string codec);

        const std::map<std::string, struct stream_entry *> stream_entries();
    private:
        Glib::RefPtr<Glib::MainLoop> m_mainloop;

        noPollCtx       *m_nopoll_context;
        noPollConn      *m_nopoll_listener = nullptr;

        std::list<Client*>   m_clients;
        int m_vidsource_fd = -1;
        int m_stream_listen_fd = -1;
        pthread_t m_handler_tid;
        pthread_mutex_t m_client_mutex;
        std::map<std::string, struct stream_entry *> m_stream_entries;

        bool on_new_socket_feed (const Glib::IOCondition);
        bool on_new_websocket_client (const Glib::IOCondition);
        void close_feed_socket ();
        static void *handle_incoming_feed (void *context);
        void close_websocket ();
        void on_disconnect_websocket_client (Client *client);
        void add_client (Client *client);
        void remove_client (Client *client);
    };

    // Input struct to the threat listening
    // to outputs from the video stream
    typedef struct VidListen {
        int fd;
        void *server;
    } VidListen_t;
}

#endif // JVH_SERVER_H
