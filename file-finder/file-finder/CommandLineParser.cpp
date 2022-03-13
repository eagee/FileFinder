#include "CommandLineParser.h"
#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <conio.h>

using namespace std;
using namespace filesystem;
using namespace fileFinder;

void CommandLineParser::ParseCommandLine(int argc, char *argv[])
{
    if (argc <= 1)
    {
        m_errorString = "Error: " + STR_PLEASE_SPECIFY + "\n" + STR_SAMPLE_USAGE;
    }
    else if (argc == 2)
    {
        m_errorString = "Error: " + STR_PLEASE_SPECIFY + "\n" + STR_SAMPLE_USAGE;
    }
    else // if (argc >= 3)
    {
        // Ensure we have valid parameters, and then parse the parameters accordingly.
        if (!exists(argv[1]))
        {
            m_errorString = "Error: The path specified does not exist.\n" + STR_SAMPLE_USAGE;
            return;
        }
        if (!is_directory(argv[1]))
        {
            m_errorString = "Error: The path specified is not a directory.\n" + STR_SAMPLE_USAGE;
            return;
        }

        m_path = argv[1];

        for (int ix = 2; ix < argc; ix++)
        {
            try
            {
                m_needles.push_back(argv[ix]);
            }
            catch (const std::bad_alloc &ex)
            {
                std::cout << " Error bad allocation caught in " << __FILE__ << " at line " << __LINE__ << endl;
                std::cout << " Exception: " << ex.what() << endl;
                std::terminate();
            }
        }

        m_isValid = true;
    }
}

CommandLineParser::CommandLineParser(int argc, char *argv[])
{
    ParseCommandLine(argc, argv);
}

bool CommandLineParser::IsValid() const
{
    return m_isValid;
}

std::string CommandLineParser::Path() const
{
    return m_path;
}

std::vector<std::string> CommandLineParser::Needles() const
{
    return m_needles;
}

std::string CommandLineParser::ErrorString()  const
{
    return m_errorString;
}
