#include <iostream>
#include <unistd.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include "server.h"
#include <sys/stat.h>
#include "stream_encode.h"
#include <stdlib.h>
#include "client_websocket.h"
#include "client_tcp.h"
#include "client.h"
#include <iostream>
#include "net_stream.h"

using namespace jvh;

Server::Server ()
{
    m_mainloop = Glib::MainLoop::create (true);
    // TODO: Check error
    pthread_mutex_init (&m_client_mutex, NULL);
}

Server::~Server ()
{
    nopoll_ctx_unref (m_nopoll_context);
    for (auto socket: m_open_sockets)
    {
        close (socket);
    }
}

void
Server::serve_websocket (const uint32_t client_port)
{
    std::string p (std::to_string(client_port));

    m_nopoll_context = nopoll_ctx_new ();
    if (m_nopoll_context == NULL)
    {
        throw std::runtime_error ("Failed to create noPoll context.");
    }

    m_nopoll_listener = nopoll_listener_new (m_nopoll_context, "0.0.0.0", p.c_str());
    if (nopoll_conn_is_ok (m_nopoll_listener) == false)
    {
        throw std::runtime_error ("Failed to start service on port " + p);
    }

    Glib::signal_io ().connect (sigc::mem_fun (*this, &Server::on_new_websocket_client),
                                nopoll_conn_socket (m_nopoll_listener),
                                Glib::IO_IN | Glib::IO_HUP);
}

void
Server::serve_streams_incoming (const uint32_t feed_port)
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
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof (serv_addr)) < 0)
    {
        close (sockfd);
        throw std::runtime_error ("Failed to bind socket on port ");
    }

    listen (sockfd, 5);

    m_open_sockets.push_back (sockfd);

    Glib::signal_io ().connect (sigc::mem_fun (*this, &Server::on_new_video_feed),
                                sockfd,
                                Glib::IO_IN | Glib::IO_HUP);
}

void
Server::serve_socket_tcp (const uint32_t port)
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
    serv_addr.sin_port = htons (port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof (serv_addr)) < 0)
    {
        close (sockfd);
        throw std::runtime_error ("Failed to bind socket on port ");
    }

    // Only caring about the one connection
    // however it may be smarter to listen on more
    listen (sockfd, 1);

    m_open_sockets.push_back (sockfd);

    Glib::signal_io ().connect (sigc::mem_fun (*this, &Server::on_new_tcp_client),
                                sockfd,
                                Glib::IO_IN | Glib::IO_HUP);
}

bool
Server::on_new_tcp_client (const Glib::IOCondition condition)
{
    struct sockaddr addr;
    socklen_t addrlen = sizeof (struct sockaddr);
    int new_fd;

    new_fd = accept (m_video_feed_fd, (struct sockaddr *)&addr, &addrlen);
    if (new_fd < 0)
    {
        // Something more descriptive maybe?
        throw std::runtime_error (strerror (errno));
    }

    Client *client = new ClientTCP (this, new_fd);

    client->listen_async ();

    add_client (client);

    return true;
}

void
Server::start_listening ()
{
    m_mainloop->run ();
}

void
Server::close_websocket ()
{
    if (m_nopoll_listener)
    {
        nopoll_conn_close (m_nopoll_listener);
    }
}

void
Server::close_feed_socket ()
{
}

void
Server::close_sockets ()
{
    // XXX: cleanup all dangling threads and class references
    if (m_mainloop)
    {
        m_mainloop->quit ();
    }
    close_websocket ();
    close_feed_socket ();
}

bool
Server::on_new_video_feed (const Glib::IOCondition)
{
    int new_conn;
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    int rc;

    new_conn = accept (m_stream_listen_fd, (struct sockaddr*)&cli_addr, &clilen);
    if (new_conn < 0)
    {
        std::cerr << "unable to accept new connection."
                  << std::endl;
        return false;
    }

    NetStream *stream = new NetStream (new_conn, this);

    stream->signal_disconnected ().connect
        (sigc::mem_fun (*this, &Server::on_disconnect_stream));

    stream->run_threaded ();

    std::shared_ptr<Stream> s (stream);

    m_active_streams["stream_encode"] = s;

    return true;
}

void
Server::on_disconnect_stream (Stream *stream)
{
    auto iter = m_active_streams.begin ();

    // XXX: please remove stream_entry as well
    while (iter != m_active_streams.end ())
    {
        // Comparing smart pointer
        // to raw pointer
        if (iter->second.get () == stream)
        {
            m_active_streams.erase (iter);
            break;
        }
        iter++;
    }
}

bool
Server::on_new_websocket_client (const Glib::IOCondition)
{
    std::cerr << "New websocket client connected." << std::endl;

    noPollConn *ws = nopoll_conn_accept (m_nopoll_context, m_nopoll_listener);
    Client *client = new ClientWebSocket (this, ws);

    client->signal_disconnected ().connect
        (sigc::mem_fun (*this, &Server::on_disconnect_websocket_client));

    // run client mainloop in new thread
    client->listen_async ();

    add_client (client);

    return true;
}

void
Server::on_disconnect_websocket_client (Client *client)
{
    std::cerr << "Websocket client disconnected" << std::endl;
    remove_client (client);
}

void
Server::remove_client(Client *client)
{
    std::unique_lock<std::mutex> lock (m_client_lock);
    m_clients.remove (client);
    delete client;
}

void
Server::add_client(Client *client)
{
    std::unique_lock<std::mutex> lock (m_client_lock);
    m_clients.push_back (client);
}

void
Server::register_stream_entry_file (std::string name, std::string filepath,
                                    std::string format, std::string codec)
{
    struct stream_entry *entry = new stream_entry;

    entry->se_name = name;
    entry->se_format = format;
    entry->se_codec = codec;
    m_stream_entries[name] = entry;
}

void
Server::stream_file_encode (std::string name, std::string filepath, std::string format,
                            uint32_t width, uint32_t height)
{
    struct stream_entry *entry = new stream_entry;

    entry->se_name = name;
    entry->se_filepath = filepath;
    entry->se_width = width;
    entry->se_height = height;
    m_stream_entries[name] = entry;

    std::shared_ptr<Stream> stream (new StreamEncoded (entry));

    m_active_streams[name] = stream;
}

void
Server::register_stream_entry (struct stream_entry *entry, enum stream_type stream_type)
{
    struct stream_entry *e = new struct stream_entry;

    switch(stream_type)
    {
    case STREAM_TYPE_ENCODE_FEED:
        e->se_name = entry->se_name;
        e->se_type = stream_type;
        e->se_format = entry->se_format;
        e->se_height = entry->se_height;
        e->se_width = entry->se_width;
        break;
    }
    add_stream_entry (e);
}

void
Server::add_stream_entry (struct stream_entry *entry)
{
    std::unique_lock<std::mutex> lock (m_stream_entries_lock);

    m_stream_entries[entry->se_name] = entry;
}

void
Server::remove_stream_entry (struct stream_entry *entry)
{
    std::unique_lock<std::mutex> lock (m_stream_entries_lock);

    m_stream_entries.erase (entry->se_name);
}

const std::map<std::string, struct stream_entry *>
Server::stream_entries ()
{
    return m_stream_entries;
}

std::shared_ptr<Stream>
Server::get_stream (std::string stream_name)
{
    return m_active_streams[stream_name];
}


