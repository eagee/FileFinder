#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <functional>
#include <atomic>
#include <thread>

namespace fileFinder
{
    /// Allows consumers to specify a, "needle" that can be found in file names in the path specified.
    /// Note: Object is designed to pass matching, "needles" back to consumer via ResultCallback and FinishedCallback to allow for mutli-threading if desired.
    class FilesystemHaystack
    {
    public:
        /// Callback definition indicates a matching file name that was found, and provides reference that allows search to be terminated
        typedef std::function<void(const std::string &match, std::atomic<bool> &terminate)> ResultCallback;
        typedef std::function<void(const std::thread::id &id)> FinishedCallback;
    
    private:
        std::string m_path {""};
        std::string m_needle {""};
        std::atomic<bool> m_terminate {false};
        ResultCallback m_resultsCallback;
        FinishedCallback m_finishedCallback;
    
    public:
    
        FilesystemHaystack() = delete;

        FilesystemHaystack(const std::string &path, const std::string &needle, ResultCallback resultscallback = nullptr, FinishedCallback finishedCallback = nullptr);
    
        /// Iterate through all files and subdirectories specified by path to find needle specified, if  results
        /// are found they will be passed via callback method, which will update whether this thread should terminate or not
        /// once it's called.
        void FindNeedles();
    };
}

