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
#include <stdlib.h>
#include "client_websocket.h"
#include "client_tcp.h"
#include <iostream>

using namespace jvh;

Server::Server ()
{
    m_mainloop = Glib::MainLoop::create (true);
    // TODO: Check error
    pthread_mutex_init (&m_client_mutex, NULL);
}

Server::~Server ()
{
    m_vsource_handler_thread.join ();
    nopoll_ctx_unref (m_nopoll_context);
    if (m_stream_listen_fd)
    {
        close (m_stream_listen_fd);
    }
    if (m_video_feed_socket)
    {
        close (m_video_feed_socket);
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
Server::video_feed_listen_setup (const uint32_t feed_port)
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

    // Only caring about the one connection
    // however it may be smarter to listen on more
    listen (sockfd, 1);

    m_stream_listen_fd = sockfd;

    Glib::signal_io ().connect (sigc::mem_fun (*this, &Server::on_new_video_source),
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

    m_video_feed_socket = sockfd;

    Glib::signal_io ().connect (sigc::mem_fun (*this, &Server::on_new_socket_tcp),
                                sockfd,
                                Glib::IO_IN | Glib::IO_HUP);
}

bool
Server::on_new_socket_tcp (const Glib::IOCondition condition)
{
    struct sockaddr addr;
    socklen_t addrlen = sizeof (struct sockaddr);
    int new_fd;


    new_fd = accept (m_video_feed_socket, (struct sockaddr *)&addr, &addrlen);
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
    close (m_vidsource_fd);
    close (m_stream_listen_fd);
}

void
Server::close_sockets ()
{
    if (m_mainloop)
    {
        m_mainloop->quit ();
    }
    close_websocket ();
    close_feed_socket ();
}

//! TODO: teardown function if feed disconnects such that
//! it can reconnect if necessary
bool
Server::on_new_video_source (const Glib::IOCondition)
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

    std::shared_ptr<NetStream> stream(new NetStream (new_conn));

    stream->signal_disconnected ().connect
        (sigc::mem_fun (*this, &Server::on_disconnect_stream));

    stream->run_threaded ();

    // Waits until the NetStream retrieves
    // information about the stream context
    auto stream_entry = stream->wait_for_stream_context ();

    m_stream_entries[stream_entry->se_name] = stream_entry;

    std::shared_ptr<Stream> s (stream);

    m_active_streams[stream_entry->se_name] = s;

    return true;
}

void
Server::on_disconnect_stream (Stream *stream)
{
    auto iter = m_active_streams.begin ();

    // XXX: please remove stream_entry aswell
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

std::list<uint32_t>
Server::get_clientids ()
{
    std::list<uint32_t> c;
    auto tot_clients = m_clients.size();

    for (auto i = 0; i < tot_clients; i++)
    {
        c.push_front (i);
    }
    return c;
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
    pthread_mutex_lock (&m_client_mutex);
    m_clients.remove (client);
    pthread_mutex_unlock (&m_client_mutex);
}

void
Server::add_client(Client *client)
{
    pthread_mutex_lock (&m_client_mutex);
    m_clients.push_back (client);
    pthread_mutex_unlock (&m_client_mutex);
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


