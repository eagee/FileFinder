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
    std::unique_ptr<CommandLineParser> parser = make_unique<CommandLineParser>(argc, argv);

    if (!parser->IsValid())
    {
        std::cout << parser->ErrorString() << endl;
        return -1;
    }

    // Just in case we want to have instructions in the command line... probably not useful for your average cli users...
    //cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    //cout << ">>> File Finder will now rescursively search \"" << Path() << "\" for matching files names. " << endl;
    //cout << ">>> Results will display every 5 seconds until all searches are complete." << endl;
    //cout << ">>> Press any key to show results so far sooner, or press 'q' to quit." << endl;
    //cout << ">>> Press any key to begin search." << endl;
    //cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    //_getch();
    //cout << ">>> Searching..." << endl;

    std::unique_ptr<ResultsMonitor> searchResultsMonitor = make_unique<ResultsMonitor>(parser->Path(), parser->Needles());
    searchResultsMonitor->SearchFilesystem();

    // I hope this is what you meant when you asked me to manually clean up memory and not rely on dtors :)
    parser.reset(nullptr);
    searchResultsMonitor.reset(nullptr);

    //cout << " It's all over now :( ";
}

