#pragma once
#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include <map>

namespace fileFinder
{
    class FilesystemHaystack;

    class ResultsMonitor
    {
    
    private:
        std::atomic<bool> m_termianteSearch {false};
        std::atomic<unsigned int> m_completeThreads {0};
        std::atomic<int> m_lastKbEntry {0};
        std::vector<std::unique_ptr<FilesystemHaystack>> m_haystacks;
        std::vector<std::unique_ptr<std::thread>> m_threads;
        std::unique_ptr<ThreadSafeQueue<std::string>> m_resultsContainer {std::make_unique<ThreadSafeQueue<std::string>>()};

        ///<summary>Initializes haystacks and threads to execute searches on them based on the number of needles specified so that they are ready to be joined. 
        ///         Only needs to be called if default ctor is used.</summary>
        void InitializeHaystacks(const std::string &path, const std::vector<std::string> &needles);

        /// <summary> Function to be run as a thread that will monitor the search result container, process keyboard input, and print results to the console </summary>
        void MonitorSearch();

        /// <summary> Convenience function that will clear out the last key pressed </summary>
        void ClearLastKeyPressed();

        /// <summary> Dumps search results to the console </summary>
        void Dump();

        /// <summary> Triggers all threads to stop searching the filesystem and exit </summary>
        void StopSearching();

        /// <summary> Processes keyboard input asynchronously in another thread and sets m_lastKbEntry to a non-null value if a key is hit</summary>
        void GetKeyboardInput();

    public:

        ResultsMonitor(const std::string &path, const std::vector<std::string> &needles);

        ///<summary>Will search the filesystem for all of the needles specified in the constructor.</summary>
        void SearchFilesystem();
    };
}
