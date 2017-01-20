#include "frame_queue.h"

void
PCQueue::enqueue (const T& elem)
{
    std::unique_lock<std::mutex> lock (m_qlock);

    m_queue.push (elem);
    m_cv.notify_one ();
}

T&
PCQueue::dequeue ()
{
    std::unique_lock<std::mutex> lock(m_qlock);
    while (m_queue.empty ())
    {
        m_cv.wait (m_qlock);
    }
    elem = m_queue.front ()
    m_queue.pop ();
}

uint64_t
PCQueue::size ()
{
    std::unique_lock <std::mutex> lock (m_lock);
    return m_queue.size ();
}
