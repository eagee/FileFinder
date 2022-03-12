#include <algorithm>
#include <cctype>
#include <iostream>
#include <future>
#include <chrono>
#include <conio.h>
#include "FileNames.h"
#include "FileNameBuffer.h"
#include "ThreadSafeQueue.h"
#include "ResultsMonitor.h"
#include "FilesystemHaystack.h"

using namespace std;
using namespace std::chrono;
using namespace fileFinder;

void ResultsMonitor::InitializeHaystacksAndBuffer(const std::string &path, const std::vector<std::string> &needles)
{
    // Set up our FileSystemHaystacks with a thread for each substring(needle) that we want to find in them
    for (auto needle : needles)
    {
        auto newHaystack = std::make_unique<FilesystemHaystack>(path, needle, 
            
            /// Implements @see FilesystemHaystack::ResultsCallback which will enqueue any needles we've found in the haystack into our results.
            [this](const std::string &match)
            {
                m_resultsContainer->Enqueue(match);
            },

            // Implements @see FilesytemHaystack::FinishedBufferCallback which is triggered each time a haystack finishes processing a buffer.
            // If the number of times it was processed matches the number of haystacks we're evaluating we can put it back into the
            // fileNameBuffer object for re-use, and to see if we've processed all of our total file names (in which case we can quit) :)
            [this](std::shared_ptr<FileNames> buffer)
            {
                std::unique_lock<std::mutex> lock(m_bufferFinishedMutex);

                buffer->ProcessedCount++;
                //std::cout << " Buffer ID:" << buffer->ID << " processed with count: " << buffer->ProcessedCount << endl;

                if (buffer->ProcessedCount >= m_haystacks.size())
                {
                    m_fileNameBuffer->EnqueueProcessedBuffer(buffer);
                    if (m_fileNameBuffer->AllFileNamesHaveBeenProcessed())
                    {
                        Stop();
                    }
                }
            }
        );

        m_haystacks.push_back(std::move(newHaystack));
    }

    // Initialize our FileNameBuffer object so that it's set up to search through the filesystem and find all the file names
    // specified, passing them to each of our haystack objects
    m_fileNameBuffer = std::make_unique<FileNameBuffer>(path, 
        // Implements @see FileNameBuffer::BufferReadyCallback, which takes a shared, populated buffer pointer and passes it to each ouf our
        // haystacks for searching.
        [this](std::shared_ptr<FileNames> buffer)
        {
            for (auto &haystack : m_haystacks)
            {
                haystack->EnqueueBufferToProcess(buffer);
            }
        }
    );

}

ResultsMonitor::ResultsMonitor(const std::string &path, const std::vector<std::string> &needles)
{
    InitializeHaystacksAndBuffer(path, needles);
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
    }
}


void ResultsMonitor::MonitorSearch()
{
    while (!m_termianteSearch)
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
                Stop();
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
        m_totalMatches++;
        cout << m_resultsContainer->Dequeue() << endl;
    }
}

void ResultsMonitor::Stop()
{
    m_termianteSearch.exchange(true);
    m_fileNameBuffer->Stop();
    for (auto &haystack : m_haystacks)
    {
        haystack->Stop();
    }
}

void ResultsMonitor::SearchFilesystem()
{
    try
    {
        // Start a dedicated thread to periodically dump our results to the console, and monitor for user input
        auto monitorThread = std::make_unique<thread>(&ResultsMonitor::MonitorSearch, this);
        
        // Start a dedicated thread to parse all filesystem file names in the specified path into a buffer
        auto filesystemThread = std::make_unique<thread>(&FileNameBuffer::PopulateBuffers, m_fileNameBuffer.get());

        // Start a dedicated thread to find the requested needles on each of the specified haystacks
        for (auto &haystack : m_haystacks)
        {
            auto newThread = std::make_unique<thread>(&FilesystemHaystack::FindNeedles, haystack.get());
            m_haystackThreads.push_back(std::move(newThread));
        }


        // Make sure we've joined all of our threads in the order that they will potentially finish
        if (filesystemThread->joinable())
        {
            filesystemThread->join();
        }

        for (auto &thread : m_haystackThreads)
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

const int64_t fileFinder::ResultsMonitor::TotalMatches()
{
    return m_totalMatches;
}
