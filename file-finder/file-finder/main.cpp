#include <iostream>
#include <thread>
#include <mutex>
#include <conio.h>
#include "FileNames.h"
#include "FileNameBuffer.h"
#include "ThreadSafeQueue.h"
#include "CommandLineParser.h"
#include "FilesystemHaystack.h"
#include "ResultsMonitor.h"

using namespace std;
using namespace fileFinder;

int main(int argc, char *argv[])
{
    std::unique_ptr<CommandLineParser> parser = make_unique<CommandLineParser>(argc, argv);

    if (!parser->IsValid())
    {
        std::cout << parser->ErrorString() << endl;
        return -1;
    }

    // Since we have users typing 'dump' or 'quit' from the command line, I thought a little exposition made sense...
    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    cout << ">>> File Finder will now rescursively search \"" << parser->Path() << "\" for matching files names. " << endl;
    cout << ">>> Results will display every 5 seconds until all searches are complete." << endl;
    cout << ">>> Press any key to show results so far sooner, or press 'q' to quit." << endl;
    cout << ">>> Press any key to begin search." << endl;
    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    _getch();
    cout << ">>> Searching..." << endl;

    std::unique_ptr<ResultsMonitor> searchResultsMonitor = make_unique<ResultsMonitor>(parser->Path(), parser->Needles());
    searchResultsMonitor->SearchFilesystem();

    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    if (searchResultsMonitor->TerminatedEarly())
    {
        cout << ">>> Search terminated early." << endl;
    }
    else
    {
        cout << ">>> Search complete!" << endl;
    }
    cout << ">>> Total matches: " << searchResultsMonitor->TotalMatches() << endl;
    
    // I hope this is what you meant when you asked me to manually clean up memory and not rely on dtors :)
    parser.reset(nullptr);
    searchResultsMonitor.reset(nullptr);

    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;

}

