// file-finder.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <thread>
#include <mutex>
#include "ThreadSafeQueue.h"
#include "CommandLineParser.h"
#include "FilesystemHaystack.h"
#include "ResultsMonitor.h"

using namespace std;
using namespace fileFinder;

std::mutex g_testMutex;

int main(int argc, char *argv[])
{
    CommandLineParser parser(argc, argv);

    if (!parser.IsValid())
    {
        std::cout << parser.ErrorString() << endl;
    }

    cout << ">>> Searching Path: " << parser.Path() << endl;
    cout << ">>> Total Needles to find in Filesystem Haystack: " << parser.Needles().size() << endl;
    cout << ">>> Program will show results every 10 seconds. Press any key to show results immediately, or press 'q' to quit." << endl;
    ResultsMonitor searchResultsMonitor(parser.Path(), parser.Needles());
    searchResultsMonitor.SearchFilesystem();

    cout << ">>> Search operation complete!" << endl;
}

