#ifndef JVH_SERVER_H
#define JVH_SERVER_H

#include <nopoll.h>
#include <list>
#include <memory>
#include <map>
#include <videostream.pb.h>
#include <glibmm/main.h>
#include <thread>
#include <mutex>
#include "client.h"

namespace jvh
{
    class Server
    {
    friend class NetStream;

    public:
        Server ();
        ~Server ();

        void serve_websocket (const uint32_t client_port);

        void stream_file_encode (std::string name, std::string filepath, std::string format,
                                uint32_t width, uint32_t height);

        void serve_socket_tcp (const uint32_t feed_port);

        void close_sockets ();


        void start_listening ();

        //! Send a protobuff message to the client on id id
        int send_message(int client_id, videostream::ToClient m);

        void serve_streams_incoming (const uint32_t port);

        std::shared_ptr<Stream> get_stream (std::string stream_name);

        void register_stream_entry_file (std::string name, std::string filepath,
                                         std::string format, std::string codec);

        const std::map<std::string, struct stream_entry *> stream_entries();

    private:
        Glib::RefPtr<Glib::MainLoop> m_mainloop;

        noPollCtx       *m_nopoll_context;
        noPollConn      *m_nopoll_listener = nullptr;
        std::map<std::string, std::shared_ptr<Stream>> m_active_streams;

        //! Handles incoming raw video from either
        //! file or socket
        std::thread m_vsource_handler_thread;
        std::list<Client*>   m_clients;
        std::vector<int> m_open_sockets;
        std::mutex m_stream_entries_lock;
        std::mutex m_client_lock;

        int m_stream_listen_fd;
        int m_video_feed_fd;

        pthread_mutex_t m_client_mutex;

        std::map<std::string, struct stream_entry *> m_stream_entries;

        bool on_new_video_feed (const Glib::IOCondition);
        void remove_stream_entry (struct stream_entry *entry);
        void add_stream_entry (struct stream_entry *entry);
        void register_stream_entry (struct stream_entry *entry, enum stream_type stream_type);
        bool on_new_tcp_client (const Glib::IOCondition);
        bool on_new_websocket_client (const Glib::IOCondition);

        void on_disconnect_stream (Stream *stream);

        void serve_socket (const uint32_t port);
        void close_feed_socket ();
        void handle_incoming_feed ();
        void close_websocket ();
        void on_disconnect_websocket_client (Client *client);
        void add_client (Client *client);
        void remove_client (Client *client);
    };
}

#endif // JVH_SERVER_H
