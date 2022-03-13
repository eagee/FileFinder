#pragma once
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <string>
#include <filesystem>

namespace fileFinder
{
    template <typename T>
    class ThreadSafeQueue;
    struct FileNames;

    /// FileNameBuffer wraps std::filesystem::recursive_directory_iterator to provide a list of read-only buffers (in a callback) as the specified path is searched for file names, which
    /// can the be passed to one or more consuming threads for searching. Buffers should re-enqueued once they have been searched in order to allow a pool of buffers to be
    /// reused (and to reduce memory fragmentation).
    class FileNameBuffer
    {
    public:
        /// Callback definition triggered when a filesystem buffer is ready for processing by one or more consumers
        typedef std::function<void(std::shared_ptr<FileNames>)> BufferReadyCallback;

    private:
        const int INITIAL_BUFFER_COUT{ 64 };
        std::string m_path;
        std::atomic<bool> m_finishedPopulating{ false };
        std::unique_ptr<ThreadSafeQueue<std::shared_ptr<FileNames>>> m_availableBuffers{std::make_unique<ThreadSafeQueue<std::shared_ptr<FileNames>>>()};
        std::atomic<int> m_totalBuffersCreated{ 0 };
        std::filesystem::recursive_directory_iterator m_it;
        BufferReadyCallback m_bufferReadyCallback;
        std::atomic<bool> m_terminateEarly{ false };
        
        /// Returns the next available buffer for populating, if there are no buffers left to populate then a new buffer will be
        /// allocated and the total number of buffers created will be increased by one.
        std::shared_ptr<FileNames> GetNextAvailableBuffer();

    public:

        FileNameBuffer() = delete;

        /// Accepts a path to generate buffers from by iterating the path recursively and pulling out all of the file names contained in the directory.
        /// Allows consuming object to specify code that will be triggered in a callback whenever a new buffer of file names is ready for processing.
        FileNameBuffer(const std::string &path, BufferReadyCallback bufferReadyCallback = nullptr);

        /// Copying this object is not part of our use case, so we'll set it up as non-copyable
        FileNameBuffer& operator=(const FileNameBuffer&) = delete;

        /// Initializes the default number of buffers set by INITIAL_BUFFER_COUNT to provide enough initial buffer space to continue producing
        /// while the search threads consume the buffers we populate (though buffers can grow if we are not IO bound)
        void InitializeBuffers();

        /// Indicates the total number of buffers that were created during the lifetime of FileNameBuffer
        int TotalBuffersCreated();

        /// Method that can be called in a separate thread to populate buffers with file names found recursively in the path specified in the constructor.
        /// Will trigger BufferReadyCallback passed into the ctor when a new buffer is available for processing.
        void PopulateBuffers();

        /// Calling stop will cause the PopulateBuffers method to terminate early.
        void Stop();

        /// Allows a previously used file name buffer to be re-used to prevent constantly allocating more space for file names as they're processed (assuming the app isn't IO bound)
        /// This method is also important to call b/c FileNameBuffer uses it to track if all buffers have been processed in @see FileNameBuffer::AllFileNamesHaveBeenProcessed()
        void EnqueueProcessedBuffer(std::shared_ptr<FileNames> buffer);

        /// Returns true if all buffers have been populated and returned via @see FileNameBuffer::EnqueueProcessedBuffer and PopulateBuffersForPath has finished iterating through all of the possible
        /// files in the path.
        bool AllFileNamesHaveBeenProcessed();

    };
}
