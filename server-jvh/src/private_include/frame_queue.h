#ifndef JVH_FRAME_QUEUE_H
#ifndef JVH_FRAME_QUEUE_H

namespace jvh
{

    template <typename T> class StreamQueue
    {
    public:
        StreamQueue ();
        ~StreamQueue ();
    private:
        std::map<int, std::queue<T>> m_client_streams;
    };
}

#endif
