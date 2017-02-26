#ifndef JVH_PB_COMM_TCP_H
#define JVH_PB_COMM_TCP_H

#include <iostream>
#include "videostream.pb.h"

namespace jvh
{
    class ProtoTCP
    {
    public:
        ProtoTCP (int socket);
        ~ProtoTCP ();

        template <typename T>
        bool
        send (T& message)
        {
            std::string str;

            message.SerializeToString (&str);
            uint32_t message_length = str.size ();

            if (write_to_socket (sizeof (uint32_t), (char *) &message_length) < 0)
            {
                std::cerr << strerror (errno) << std::endl;
                return false;
            }

            if (write_to_socket (str.size (), (char *) str.c_str ()) < 0)
            {
                std::cerr << strerror (errno) << std::endl;
                return false;
            }
            return true;
        }

        template <typename T>
        bool
        receive (T& message)
        {
            auto msg_size = read_message_header ();
            if (msg_size < 0)
            {
                return false;
            }

            char *in_message = new char[msg_size];

            if (read_from_socket (msg_size, in_message) < 0)
            {
                delete []in_message;
                return false;
            }

            if (message.ParseFromArray ((const char *)in_message, msg_size) == false)
            {
                std::cerr << "Failed to parse pbmessage" << std::endl;
                return false;
            }

            delete []in_message;
            return true;
        }

        bool incoming_timeout (unsigned int m_sec);

    private:
        int m_socket;
        fd_set readfds;
        int read_from_socket (uint32_t read_size, char *buffer);
        uint32_t read_message_header ();
        int write_to_socket (uint32_t write_size, char *buffer);

    };


}

#endif

