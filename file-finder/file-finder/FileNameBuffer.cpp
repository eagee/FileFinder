#include <vector>
#include "FileNames.h"
#include "FileNameBuffer.h"
#include "ThreadSafeQueue.h"

using namespace std;
using namespace filesystem;
using namespace fileFinder;

fileFinder::FileNameBuffer::FileNameBuffer(const std::string &path, BufferReadyCallback bufferReadyCallback /*= nullptr*/):
        m_it(recursive_directory_iterator(path, directory_options::skip_permission_denied)),
        m_path(path),
        m_bufferReadyCallback(bufferReadyCallback)
{
    InitializeBuffers();
}

void fileFinder::FileNameBuffer::InitializeBuffers()
{
    for (int ix = 0; ix < INITIAL_BUFFER_COUT; ix++)
    {
        std::shared_ptr<FileNames> newBuffer = make_shared<FileNames>();
        newBuffer->ID = ix;
        m_availableBuffers->Enqueue(newBuffer);
        m_totalBuffersCreated++;
    }
}

int fileFinder::FileNameBuffer::TotalBuffersCreated()
{
    return m_totalBuffersCreated;
}

void fileFinder::FileNameBuffer::PopulateBuffers()
{
    auto currentBuffer = m_availableBuffers->Dequeue();
    while (m_it != recursive_directory_iterator() && !m_terminateEarly)
    {
        auto path = m_it->path().string();
        auto fileName = m_it->path().filename().string();

        // Populate the current buffer until we've got enough file names to pass it back to the parent object so that it can be
        // processed, and then handle dequeuing our next buffer
        currentBuffer->Buffer->push_back(fileName);
        if (currentBuffer->Buffer->size() >= FileNames::MAX_BUFFER_SIZE)
        {
            m_bufferReadyCallback(currentBuffer);
            currentBuffer = GetNextAvailableBuffer();
        }
        
        try
        {
            ++m_it;
        }
        catch (filesystem_error& err)
        {
            // Since our project has a simplifying assumption that we have access to all files and directories, we'll go ahead and end the loop if we run into an access error.
            std::cout << ">>> Error: " << err.what() << " when searching path " << m_path << std::endl;
            continue;
        }

    }
    
    // After we've finished recursively iterating through all the files in the path specified, we want to make sure we process any files remaining
    if (currentBuffer->Buffer->size() > 0)
    {
        m_bufferReadyCallback(currentBuffer);
    }
    
    // Set value to indicate we've iterated through all of the potential file names in the path, this will be used in FileNameBuffer::AllFileNamesHaveBeenProcessed()
    // to track if all of the file names we sent were processed successfully.
    m_finishedPopulating.exchange(true);
}

void fileFinder::FileNameBuffer::Stop()
{
    m_terminateEarly.exchange(true);
}

std::shared_ptr<FileNames> fileFinder::FileNameBuffer::GetNextAvailableBuffer()
{
    if (m_availableBuffers->Size() > 0)
    {
        std::shared_ptr<FileNames> buffer = m_availableBuffers->Dequeue();
        buffer->Buffer->clear();
        buffer->ProcessedCount.exchange(0);
        return buffer;
    }
    else
    {
        m_totalBuffersCreated++;
        auto newBuffer = std::make_shared<FileNames>();
        newBuffer->ID = 99999;
        return std::make_shared<FileNames>();
    }
}

void fileFinder::FileNameBuffer::EnqueueProcessedBuffer(std::shared_ptr<FileNames> buffer)
{
    m_availableBuffers->Enqueue(buffer);
}

bool fileFinder::FileNameBuffer::AllFileNamesHaveBeenProcessed()
{
    if (m_finishedPopulating)
    {
        if (m_availableBuffers->Size() == m_totalBuffersCreated)
        {
            return true;
        }
    }
    return false;
}

