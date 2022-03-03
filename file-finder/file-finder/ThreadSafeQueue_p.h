#pragma once
#include <string>

namespace fileFinder {
    template <class T>
    void ThreadSafeQueue<T>::Enqueue(T t)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(t);
        m_condition.notify_one();
    }

    template <class T>
    T ThreadSafeQueue<T>::Dequeue()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            m_condition.wait(lock);
        }
        T val = m_queue.front();
        m_queue.pop();
        return val;
    }

    template <class T>
    size_t ThreadSafeQueue<T>::Size()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }
}