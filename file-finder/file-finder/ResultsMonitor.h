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
        std::atomic<bool> m_termianteSearch = false;
        std::atomic<unsigned int> m_completeThreads = 0;
        std::vector<std::unique_ptr<FilesystemHaystack>> m_haystacks;
        std::vector<std::unique_ptr<std::thread>> m_threads;
        std::unique_ptr<ThreadSafeQueue<std::string>> m_resultsContainer;
    
        ///<summary>Initializes haystacks and threads to execute searches on them based on the number of needles specified so that they are ready to be joined</summary>
        void InitializeHaystacks(const std::string &path, const std::vector<std::string> &needles);
    public:
    
        ResultsMonitor(const std::string &path, const std::vector<std::string> &needles);
    
        void MonitorSearch();
    
        void Dump();
    
        void StopSearching();
    
        void SearchFilesystem();
    
        ~ResultsMonitor();
    };
}
