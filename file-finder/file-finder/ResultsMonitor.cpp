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
    InitializeHaystacks(path, needles);
}

void ResultsMonitor::GetKeyboardInput()
{
    bool timeoutExpired = false;
    system_clock::time_point start = system_clock::now();
    duration<double> timeout = std::chrono::seconds(5);

    while (!timeoutExpired && !m_lastKbEntry)
    {
        // Check our duration to see if we've timed out waiting on input.
        duration<double> elapsed = system_clock::now() - start;
        if (elapsed >= timeout)
        {
            timeoutExpired = true;
        }

        // Check to see if the keyboard was interacted with and if so assign it to our member variable
        if (_kbhit())
        {
            m_lastKbEntry.exchange(_getch());
        }
        
        // Debug output :-)
        //cout << " kb: " << m_lastKbEntry << " done: " << m_completeThreads << " of: " << m_haystacks.size() << " elapsed: " << elapsed.count() << " of " << timeout.count() << endl;
    }
}


void ResultsMonitor::MonitorSearch()
{
    while (m_completeThreads < m_haystacks.size())
    {
        // Process user input until 5 seconds have passed or the user has interacted with the keyboard
        auto kbThread = std::make_unique<thread>(&ResultsMonitor::GetKeyboardInput, this);
        if (kbThread->joinable())
        {
            kbThread->join();
        }

        if (m_lastKbEntry)
        {
            if (tolower(m_lastKbEntry) == 'q')
            {
                StopSearching();
            }
        }

        ClearLastKeyPressed();

        // Once we've processed input, we can go ahead and dump whatever records are still available.
        Dump();
    }

    // Make sure we've dumped whatever records are still remaining after all of our threads have finished running
    Dump();
}

void ResultsMonitor::Dump()
{
    while (m_resultsContainer->Size())
    {
        cout << m_resultsContainer->Dequeue() << endl;
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

void fileFinder::ResultsMonitor::ClearLastKeyPressed()
{
    m_lastKbEntry.exchange(0);
}
