#include "SynchronizedDirectoryIterator.h"

using namespace std;
using namespace filesystem;
using namespace fileFinder;

fileFinder::SynchronizedDirectoryIterator::SynchronizedDirectoryIterator(const string &path, const int threadsRequiredPerIncrement) :
    m_threadsRequiredPerIncrement(threadsRequiredPerIncrement),
    m_referenceCount(threadsRequiredPerIncrement),
    m_it(recursive_directory_iterator(path, directory_options::skip_permission_denied))
{

}

void SynchronizedDirectoryIterator::operator++()
{
    m_referenceCount--;
    if (m_referenceCount == 0)
    {
        std::unique_lock<std::mutex> writeLock(m_mutex);
        try
        {
            ++m_it;
        }
        catch (std::filesystem::filesystem_error &err)
        {
            throw(err);
        }
        m_referenceCount = m_threadsRequiredPerIncrement;
        writeLock.unlock();
        //lock.unlock();
        m_condition.notify_all();
    }
    else
    {
        std::unique_lock<std::mutex> readLock(m_mutex);
        m_condition.wait(readLock);
    }
}

std::string SynchronizedDirectoryIterator::Path()
{
    return m_it->path().string();
}

std::string SynchronizedDirectoryIterator::FileName()
{
    return m_it->path().filename().string();
}

bool SynchronizedDirectoryIterator::Finished()
{
    return (m_it == std::filesystem::recursive_directory_iterator());
}
