#ifndef JVH_FRAME_QUEUE_H
#ifndef JVH_FRAME_QUEUE_H

#include <mutex>
#include <condition_variable>

namespace jvh
{

    template <typename T> class PCQeueue
    {
    public:
        StreamQueue ();
        ~StreamQueue ();

        void enqueue (const T& elem);
        T& dequeue ();

    private:
        std::queue<T> m_queue;
        std::mutex m_qlock;
        std::condition_variable m_cv;
    };
}

#endif
