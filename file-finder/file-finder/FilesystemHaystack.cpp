#include "FilesystemHaystack.h"
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <cassert>

using namespace std;
using namespace filesystem;
using namespace fileFinder;

FilesystemHaystack::FilesystemHaystack(const std::string &path, const std::string &needle, ResultCallback resultsCallback /*= nullptr*/, FinishedCallback finishedCallback /*= nullptr*/) :
    m_path(path), m_needle(needle), m_resultsCallback(resultsCallback), m_finishedCallback(finishedCallback)
{
    assert(m_resultsCallback != nullptr);
    assert(m_finishedCallback != nullptr);
}

void FilesystemHaystack::FindNeedles()
{
    auto it = recursive_directory_iterator(m_path);
    while(it != recursive_directory_iterator())
    {
        auto path = it->path().string();
        auto fileName = it->path().filename().string();
        
        // Search the fileName string using boyer_moore algorithm for pattern matching
        auto searchIt = std::search(fileName.begin(), fileName.end(), std::boyer_moore_searcher(m_needle.begin(), m_needle.end()));
        
        // If a match for the needle is found in our fileName then trigger a callback to add the fileName to our container
        // otherwise pass an empty string to the callback so that we can still see if  the user wishes to terminate
        if (searchIt != fileName.end())
        {
            m_resultsCallback(fileName, std::ref(m_terminate));
        }
        else
        {
            m_resultsCallback("", std::ref(m_terminate));
        }
        
        // If our parent object has indicated it wants us to terminate the search, we'll terminate hte loop.
        if (m_terminate == true)
        {
            break;
        }

        try
        {
            ++it;
        }
        catch (filesystem_error& err)
        {
            // Since our project has a simplifying assumption that we have access to all files and directories, we'll go ahead and end the loop if we run into an access error.
            std::cout << ">>> Error: " << err.what() << " when searching path " << path << std::endl;
            break;
        }
        
    }
    m_finishedCallback(std::this_thread::get_id());
}
