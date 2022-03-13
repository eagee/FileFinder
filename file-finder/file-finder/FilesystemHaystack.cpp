#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <cassert>
#include "FileNames.h"
#include "ThreadSafeQueue.h"
#include "FilesystemHaystack.h"

using namespace std;
using namespace filesystem;
using namespace fileFinder;

FilesystemHaystack::FilesystemHaystack(const std::string &path, const std::string &needle, ResultsCallback resultsCallback /*= nullptr*/, FinishedBufferCallback finishedCallback /*= nullptr*/) :
    m_path(path), 
    m_needle(needle),
    m_resultsCallback(resultsCallback),
    m_finishedCallback(finishedCallback)
{
    assert(m_resultsCallback != nullptr);
    assert(m_finishedCallback != nullptr);
}

void FilesystemHaystack::FindNeedles()
{
    // Loop until m_terminate is set to true in m_resultsCallback.
    while(!m_terminateSearch)
    {
        if (m_buffersToProcess->Size() > 0)
        {
            auto readOnlyBuffer = m_buffersToProcess->Dequeue();
            for (auto name : *readOnlyBuffer->Buffer)
            {
                try 
                {
                    // Search the fileName string using boyer_moore algorithm for pattern matching
                    auto searchIt = std::search(name.begin(), name.end(), std::boyer_moore_searcher(m_needle.begin(), m_needle.end()));
                    
                    // If a match for the needle is found in our fileName then trigger a callback to add the fileName to our container
                    // otherwise pass an empty string to the callback so that we can still see if the user wishes to terminate
                    if (searchIt != name.end())
                    {
                        m_resultsCallback(name);
                    }
                }
                catch (const std::bad_alloc &ex)
                {
                    std::cout << " Error bad allocation caught in " << __FILE__ << " at line " << __LINE__ << endl;
                    std::cout << " Exception: " << ex.what() << endl;
                    std::terminate();
                }

                
                
                if (m_terminateSearch)
                {
                    break;
                }
            }
            m_finishedCallback(readOnlyBuffer);
        }

        // TODO: REPLACE THIS SLEEP COMMAND WITH A WAIT CONDITION!
        std::this_thread::sleep_for(1ms);
    }
}

void fileFinder::FilesystemHaystack::EnqueueBufferToProcess(std::shared_ptr<FileNames> buffer)
{
    m_buffersToProcess->Enqueue(buffer);
}

void fileFinder::FilesystemHaystack::Stop()
{
    m_terminateSearch.exchange(true);
}
