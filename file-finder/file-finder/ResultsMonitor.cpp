#include <algorithm>
#include <cctype>
#include <iostream>
#include <future>
#include <chrono>
#include <conio.h>
#include "ThreadSafeQueue.h"
#include "ResultsMonitor.h"
#include "FilesystemHaystack.h"

using namespace std;
using namespace std::chrono;
using namespace fileFinder;

void ResultsMonitor::InitializeHaystacks(const std::string &path, const std::vector<std::string> &needles)
{
    for (auto needle : needles)
    {
        auto newHaystack = std::make_unique<FilesystemHaystack>(path, needle,
            [this](const std::string &match, std::atomic<bool> &terminate)
            {
                if (match != "")
                {
                    m_resultsContainer->Enqueue(match);
                }
                terminate.exchange(m_termianteSearch);
            },
            [this](const std::thread::id id)
            {
                m_completeThreads++;
            } 
        );
        m_haystacks.push_back(std::move(newHaystack));
    }
}

ResultsMonitor::ResultsMonitor(const std::string &path, const std::vector<std::string> &needles)
{
    m_resultsContainer = std::make_unique<ThreadSafeQueue<std::string>>();
    InitializeHaystacks(path, needles);
}

void ResultsMonitor::MonitorSearch()
{
    while (m_completeThreads < m_haystacks.size())
    {
        // Request user input using std::async
        system_clock::time_point fiveSecondTimeout = system_clock::now() + std::chrono::seconds(5);
        future<char> asyncUserInput = async(launch::async, 
            [this]() 
            {
                char userInput;
                userInput = _getch(); 
                return userInput;
            }
        );
        future_status status = asyncUserInput.wait_until(fiveSecondTimeout);

        // If we received user input, act on it, (tolower transform should probably be implemented in a utility class or method)
        if (status == std::future_status::ready)
        {
            char result = asyncUserInput.get();
            
            if (tolower(result) == 'q')
            {
                StopSearching();
            }
        }
        // Once we've processed input, we can go ahead and dump whatever records are still available.
        Dump();
    }

    // Make sure we've dumped whatever records are still remaining if we're done monitoring our threads
    Dump();
}

void ResultsMonitor::Dump()
{
    while (m_resultsContainer->size())
    {
        string output = m_resultsContainer->Dequeue();
        cout << output << endl;
    }
}

void ResultsMonitor::StopSearching()
{
    std::cout << ">>> Quit requested, terminiated threads." << endl;
    m_termianteSearch.exchange(true);
}

void ResultsMonitor::SearchFilesystem()
{
    try
    {
        // Start a dedicated thread to periodically dump our results to the console, and monitor for user input
        auto monitorThread = std::make_unique<thread>(&ResultsMonitor::MonitorSearch, this);
    
        // Start a dedicated thread to find the requested needles on each of the specified haystacks
        for (auto &haystack : m_haystacks)
        {
            auto newThread = std::make_unique<thread>(&FilesystemHaystack::FindNeedles, haystack.get());
            m_threads.push_back(std::move(newThread));
        }
    
        // Make sure we've joined all of our threads so that this method blocks until we're finished.
        for (auto &thread : m_threads)
        {
            if (thread->joinable())
            {
                thread->join();
            }
        }
    
        if (monitorThread->joinable())
        {
            monitorThread->join();
        }
    }
    catch (const std::exception &ex)
    {
        cout << "Error, thread exited with exception: " << ex.what() << endl;
        exit(1);
    }
}

ResultsMonitor::~ResultsMonitor()
{
    
}
