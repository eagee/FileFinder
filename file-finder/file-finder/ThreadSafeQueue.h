#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

namespace fileFinder
{
    /// <summary> Template class capable of performing queuing operations safely in a threaded environment </summary>
    template <typename T>
    class ThreadSafeQueue
    {
    private:
        std::mutex m_mutex;
        std::condition_variable m_condition;
        std::queue<T> m_queue;

    public:
        ThreadSafeQueue() = default;

        /// <summary> Enqueue an element of type T, will unblock Dequeue operation if blocked waiting on an item. </summary>
        void Enqueue(T t);

        /// <summary> Dequeue an element of type T, but sleep the thread if no elements exist in the queue yet until Enqueue is called. </summary>
        T Dequeue();

        /// <summary> Returns the number of items in the queue</summary>
        int Size();
    };

    typedef class ThreadSafeQueue<std::string> TSQString;
    typedef class ThreadSafeQueue<int> TSQInt;
    typedef class ThreadSafeQueue<double> TSQDouble;
}

#include "ThreadSafeQueue_p.h"