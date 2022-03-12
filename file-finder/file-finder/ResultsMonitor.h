#pragma once
#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include <map>

namespace fileFinder
{
    class FilesystemHaystack;
    class FileNameBuffer;
    
    /// ResultsMonitor monitors filesystem-search for each, "needle" requested by the consumer as well as keyboard input while the searches complete.
    /// Note: This object will dump search results to the console every 5 seconds or when user input is received, ending search when 'q' is pressed.
    class ResultsMonitor
    {
    
    private:
        std::mutex m_bufferFinishedMutex;
        std::atomic<bool> m_termianteSearch {false};
        //std::atomic<unsigned int> m_completeThreads {0};
        std::atomic<int> m_lastKbEntry {0};
        std::vector<std::unique_ptr<FilesystemHaystack>> m_haystacks;
        std::vector<std::unique_ptr<std::thread>> m_haystackThreads;
        std::unique_ptr<ThreadSafeQueue<std::string>> m_resultsContainer {std::make_unique<ThreadSafeQueue<std::string>>()};
        std::atomic<int64_t> m_totalMatches {0};
        std::unique_ptr<FileNameBuffer> m_fileNameBuffer;
        bool m_terminatedEarly{ false };
        
        /// Initializes FileNameBuffer that will populate FilesytemHaystack objects with file names recursively from the directory specified, as well as 
        /// the haystacks and threads used to search them based on the number of needles specified.
        void InitializeHaystacksAndBuffer(const std::string &path, const std::vector<std::string> &needles);

        ///  Function to be run as a thread that will monitor the search result container, process keyboard input, and print results to the console
        void MonitorSearch();

        ///  Convenience function that will clear out the last key pressed 
        void ClearLastKeyPressed();

        ///  Dumps search results to the console 
        void Dump();

        ///  Triggers all threads to stop processing their results and terminates the check for keyboard input.
        void Stop();

        ///  Processes keyboard input asynchronously in another thread and sets m_lastKbEntry to a non-null value if a key is hit
        void GetKeyboardInput();

    public:

        ResultsMonitor(const std::string &path, const std::vector<std::string> &needles);

        /// Will search the filesystem for all of the needles specified in the constructor.
        void SearchFilesystem();

        /// Indicates whether the search was terminated early or not.
        bool TerminatedEarly() const { return m_terminatedEarly; }

        /// Will indicate the total number of matching files found during the search.
        const int64_t TotalMatches();
    };
}
