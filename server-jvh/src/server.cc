#include <iostream>
#include <unistd.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include "server.h"
#include <stdlib.h>

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
Server::serve_feed_socket (const uint32_t feed_port)
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

    Glib::signal_io ().connect (sigc::mem_fun (*this, &Server::on_new_socket_feed),
                                sockfd,
                                Glib::IO_IN | Glib::IO_HUP);
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
    pthread_kill(m_handler_tid, SIGKILL);
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

bool
Server::on_new_socket_feed (const Glib::IOCondition)
{
    int readfd;
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    int rc;

    // We only allow one connction on the video input
    // source port
    if (m_vidsource_fd != -1)
    {
        std::cerr << "Rejecting new video source input connection."
                  << std::endl;
        return false;
    }

    std::cerr << "vstream websocket connected."
              << std::endl;

    readfd = accept (m_stream_listen_fd, (struct sockaddr*)&cli_addr, &clilen);
    if (readfd < 0)
    {
        std::cerr << "unable to accept new connection."
                  << std::endl;
        return false;
    }

    // Storing the filedes
    m_vidsource_fd = readfd;

    VidListen_t *v = new VidListen_t;
    v->server = this;
    v->fd     = readfd;

    pthread_t vstream_thread;

    rc = pthread_create (&vstream_thread, NULL,
                         &Server::handle_incoming_feed, (void*)v);
    if (rc)
    {
        std::cerr << "Unable to create thread."
                  << std::endl;
    }
    // Storing threadid
    m_handler_tid = vstream_thread;
    return true;
}

void *
Server::handle_incoming_feed (void *context)
{
    VidListen_t *v = (VidListen_t*)context;

    //videofeed::FromFeed message;
    //while (1)
    //{
    //    message.ParseFromFileDescriptor(v->fd);
    //}
}

bool
Server::on_new_websocket_client (const Glib::IOCondition)
{
    std::cerr << "New websocket client connected." << std::endl;

    noPollConn *ws = nopoll_conn_accept (m_nopoll_context, m_nopoll_listener);
    Client *client = new Client (this, ws);

    client->signal_disconnected ().connect
        (sigc::mem_fun (*this, &Server::on_disconnect_websocket_client));

    add_client (client);

    return true;
}

std::list<client>
Server::get_clientids ()
{
    std::list<client> c;
    auto tot_clients = m_clients.size();

    for (auto i = 0; i < tot_clients; i++)
    {
        c.push_front (i);
    }
    return c;
}

int
Server::send_message (client id, videostream::ToClient m)
{

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
    entry->se_filepath = filepath;
    entry->se_format = format;
    entry->se_codec = codec;

    m_stream_entries[name] = entry;
}

const std::map<std::string, struct stream_entry *>
Server::stream_entries ()
{
    return m_stream_entries;
}
