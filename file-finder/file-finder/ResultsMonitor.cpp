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
        
        try 
        {
            m_haystacks.push_back(std::move(newHaystack));
        }
        catch (const std::bad_alloc &ex)
        {
            std::cout << " Error bad allocation caught in " << __FILE__ << " at line " << __LINE__ << endl;
            std::cout << " Exception: " << ex.what() << endl;
            std::terminate();
        }
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
    while (!m_termianteSearch)
    {
        string action = "";

        // Request user input using std::async
        system_clock::time_point fiveSecondTimeout = system_clock::now() + std::chrono::seconds(5);
        future<string> asyncUserInput = async(launch::async,
            []()
            {
                string userInput;
                std::getline(std::cin, userInput);
                return userInput;
            }
        );

        future_status status = asyncUserInput.wait_until(fiveSecondTimeout);

        // If we received user input, act on it, (tolower transform should probably be implemented in a utility class or method)
        if (status == std::future_status::ready)
        {
            /// Get our result from user input and transform it to a lower case string.
            action = asyncUserInput.get();
            std::for_each(action.begin(), action.end(),
                [](char &c)
                {
                    c = ::tolower(c);
                }
            );
        }
        else if (status == std::future_status::timeout)
        {
            // If the user let the keyboard entry time out, then 5 seconds has passed and we want to, "dump" anyway.
            action = "dump";
        }

        // Check to see if the user entered, "dump" or, "quit" and act on either of those options accordingly.
        if (action == "quit")
        {
            m_nextAction.exchange(NEXT_ACTION_QUIT);
        }
        else if (action == "dump")
        {
            m_nextAction.exchange(NEXT_ACTION_DUMP);
        }
        else
        {
            m_nextAction.exchange(NEXT_ACTION_NONE);
        }
    }
}


void ResultsMonitor::MonitorKeyboardInput()
{
    while (!m_termianteSearch)
    {
        if (m_nextAction == NEXT_ACTION_NONE)
        {
            std::this_thread::sleep_for(10ms);
        }
        else if (m_nextAction == NEXT_ACTION_QUIT)
        {
            m_terminatedEarly = true;
            Stop();
            m_nextAction.exchange(NEXT_ACTION_NONE);
        }
        else if (m_nextAction == NEXT_ACTION_DUMP)
        {
            Dump();
            m_nextAction.exchange(NEXT_ACTION_NONE);
        }
    }

    // Make sure we've dumped whatever records are still remaining after the search has terminated
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
        // Start a dedicated thread to process keyboard input
        auto inputThread = std::make_unique<thread>(&ResultsMonitor::GetKeyboardInput, this);

        // Start a dedicated thread to respond to keyboard input and display results
        auto monitorThread = std::make_unique<thread>(&ResultsMonitor::MonitorKeyboardInput, this);

        // Start a dedicated thread to parse all filesystem file names in the specified path into a buffer
        auto filesystemThread = std::make_unique<thread>(&FileNameBuffer::PopulateBuffers, m_fileNameBuffer.get());

        // Start a dedicated thread to find the requested needles on each of the specified haystacks
        for (auto &haystack : m_haystacks)
        {
            auto newThread = std::make_unique<thread>(&FilesystemHaystack::FindNeedles, haystack.get());
            
            m_haystackThreads.push_back(std::move(newThread));
        }

        if (monitorThread->joinable())
        {
            monitorThread->join();
        }

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
        
        // Our input thread might be stuck waiting for getline(), if so, we can go ahead and detach it.
        if (inputThread->joinable())
        {
            inputThread->detach();
        }

    }
    catch (const std::exception &ex)
    {
        cout << "Error, thread exited with exception: " << ex.what() << endl;
        exit(1);
    }
}

const int64_t fileFinder::ResultsMonitor::TotalMatches()
{
    return m_totalMatches;
}
