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


void ShowIntroMessage(CommandLineParser &parser)
{
    // A little bit of helper text we could display
    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    cout << ">>> File Finder will now rescursively search \"" << parser.Path() << "\" for matching files names. " << endl;
    cout << ">>> Results will display every 5-10 seconds until all searches are complete." << endl;
    cout << ">>> Type 'dump' and press Enter to show records so far." << endl;
    cout << ">>> Type 'quit' and press Enter to show records so farand quit." << endl;
    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    cout << ">>> Press any key to begin search." << endl;
    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl << endl;
    _getch();
    cout << ">>> Searching..." << endl << endl;
}

void ShowClosingMessage(ResultsMonitor &searchResultsMonitor)
{
    cout << endl << endl << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    if (searchResultsMonitor.TerminatedEarly())
    {
        cout << ">>> Search terminated early." << endl;
    }
    else
    {
        cout << ">>> Search complete!" << endl;
    }
    cout << ">>> Total matches: " << searchResultsMonitor.TotalMatches() << endl;
    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
}

int main(int argc, char *argv[])
{
    std::unique_ptr<CommandLineParser> parser = make_unique<CommandLineParser>(argc, argv);

    if (!parser->IsValid())
    {
        std::cout << parser->ErrorString() << endl;
        return -1;
    }

    ShowIntroMessage(*parser);

    std::unique_ptr<ResultsMonitor> searchResultsMonitor = make_unique<ResultsMonitor>(parser->Path(), parser->Needles());
    searchResultsMonitor->SearchFilesystem();

    ShowClosingMessage(*searchResultsMonitor);


    // I hope this is what you meant when you asked me to manually clean up memory and not rely on dtors :)
    parser.reset(nullptr);
    searchResultsMonitor.reset(nullptr);
}

