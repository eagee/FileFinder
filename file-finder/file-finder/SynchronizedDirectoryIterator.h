#pragma once
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <string>
#include <filesystem>
#include <string>

namespace fileFinder
{
    /// Synchronized directory iterator wraps std::filesystem::recursive_directory_iterator using reference counting and syncrhonization objects to ensure all threads
    /// sharing the iterator are able to complete processing before iteration continues to the next filesystem element (this implementation assumes we are IO bound
    /// for filesystem access)
    class SynchronizedDirectoryIterator
    {
    private:
        std::mutex m_mutex;
        std::condition_variable m_condition;
        int m_threadsRequiredPerIncrement;
        std::atomic<int> m_referenceCount{ 0 };
        std::filesystem::recursive_directory_iterator m_it;

        // TODO: create a set of sized std::list buffer pools that can be added to based on whether the filesystem iteration is potentially outperforming our search code
        //       (which is unlikely, but hey we've got be ready for anything, right?)

    public:

        /// Accepts a path to iterate recursively, and specifies the number of threads required to attempt an increment before all threads are unblocked and an increment is allowed.
        SynchronizedDirectoryIterator(const std::string &path, const int threadsRequiredPerIncrement);

        /// Waits until the specified number of threads attempt an increment before allowing one to continue and pulsing all threads
        /// Will throw std::filesystem::filesystem_error
        void operator++();

        /// Returns the current path being pointed to by the wrapped std::filesystem::recursive_directory_iterator
        std::string Path();

        /// Returns the current file name being pointed to by the wrapped std::filesystem::recursive_directory_iterator
        std::string FileName();

        /// Indicates whether we've iterated through the entire directory structore specified by the path passed in @see SynchronizedDirectoryIterator::SynchronizedDirectoryIterator
        bool Finished();

    };
}
