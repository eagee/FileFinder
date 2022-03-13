#pragma once
#include <atomic>
#include <memory>
#include <vector>
#include <string>
#include <iostream>

namespace fileFinder
{
    /// FileNames is a struct produced by @see FileNameBuffer, and consumed by @see FilesystemHaystack via the @see ResultMonitor class.
    /// It is designed to provide a buffer containing a list of file names to process, as well as an atomic counter to track how many
    /// times it has been processed by one ore more FilesystemHaystack objects using a different thread.
    struct FileNames
    {
        static const int MAX_BUFFER_SIZE{ 1024 };
        std::atomic<int> ID{ 0 };
        std::shared_ptr<std::vector<std::string>> Buffer{ std::make_shared<std::vector<std::string>>() };
        std::atomic<size_t> ProcessedCount{ 0 };
    };
}
