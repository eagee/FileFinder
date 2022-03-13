#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <functional>
#include <atomic>
#include <thread>

namespace fileFinder
{
    class SynchronizedDirectoryIterator;
    struct FileNames;

    /// Allows consumers to specify a, "needle" that can be found in file names in the path specified.
    /// Note: Object is designed to pass matching, "needles" back to consumer via ResultCallback and FinishedCallback to allow for multi-threading if desired.
    class FilesystemHaystack
    {
    public:
        /// Callback definition indicates a matching file name that was found, and provides reference that allows search to be terminated once all buffers have been processed.
        typedef std::function<void(const std::string &match)> ResultsCallback;
        /// Callback definition which indicates when the specified thread has has finished processing the specified buffer in @see FilesystemHaystack::FindNeedles
        typedef std::function<void(std::shared_ptr<FileNames> buffer)> FinishedBufferCallback;
    
    private:
        std::string m_path {""};
        std::string m_needle {""};
        std::atomic<bool> m_terminateSearch{ false };
        ResultsCallback m_resultsCallback;
        FinishedBufferCallback m_finishedCallback;
        std::unique_ptr<ThreadSafeQueue<std::shared_ptr<FileNames>>> m_buffersToProcess{std::make_unique<ThreadSafeQueue<std::shared_ptr<FileNames>>>()};

    public:
    
        FilesystemHaystack() = delete;

        FilesystemHaystack(const std::string &path, const std::string &needle, ResultsCallback resultscallback = nullptr, FinishedBufferCallback finishedCallback = nullptr);

        /// Enqueues a buffer for processing, which will be picked up by the FindNeedles method and searched for matching substrings
        void EnqueueBufferToProcess(std::shared_ptr<FileNames> buffer);
    
        /// Iterate through all files and subdirectories specified by path to find needle specified, if  results
        /// are found they will be passed via callback method, which will update whether this thread should terminate or not
        /// once it's called.
        void FindNeedles();

        /// Calling the stop method will terminate the FindNeedles method if it's running.
        void Stop();
    };
}

