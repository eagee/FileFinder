#pragma once
#include <string>

namespace fileFinder {
    template <class T>
    void ThreadSafeQueue<T>::Enqueue(T t)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        try
        {
            m_queue.push(t);
        }
        catch (const std::bad_alloc &ex)
        {
            std::cout << " Error bad allocation caught in " << __FILE__ << " at line " << __LINE__ << std::endl;
            std::cout << " Exception: " << ex.what() << std::endl;
            std::terminate();
        }
        
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