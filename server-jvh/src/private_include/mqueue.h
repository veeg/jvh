#ifndef JVH_MESSAGE_QUEUE_H
#define JVH_MESSAGE_QUEUE_H

#include <mutex>
#include <condition_variable>
#include <queue>

namespace jvh
{
    template <typename T> class MQueue
    {
    public:
        MQueue () {}
        ~MQueue () {}

        //! Enqueue a message; no
        //! bounds to number of elements
        void enqueue (const T& elem);

        //! Dequeue - blocks if
        //! queue is empty
        T& dequeue ();
        uint64_t size ();

    private:
        std::queue<T> m_queue;
        std::mutex m_qlock;
        std::condition_variable m_cv;
    };
template <typename T>
void
MQueue<T>::enqueue (const T& elem)
{
    std::unique_lock<std::mutex> lock (m_qlock);

    m_queue.push (elem);
    m_cv.notify_one ();
}

template <typename T>
T&
MQueue<T>::dequeue ()
{
    std::unique_lock<std::mutex> lock(m_qlock);
    while (m_queue.empty ())
    {
        m_cv.wait (lock);
    }
    auto& elem = m_queue.back ();
    m_queue.pop ();
}

template <typename T>
uint64_t
MQueue<T>::size ()
{
    std::unique_lock <std::mutex> lock (m_qlock);
    return m_queue.size ();
}
}
#endif
