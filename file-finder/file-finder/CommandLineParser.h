#pragma once
#include <vector>
#include <string>
#include <filesystem>

namespace fileFinder
{
    /// Provides utility class for parsing, validating, and obtaining command line arguments passed by the user at runtime.
    /// sample usage: 
    /// CommandLineParser> parser(argc, argv);
    /// if (!parser.IsValid())
    /// {
    ///     std::cout << parser->ErrorString() << endl;
    ///     return -1;
    /// }
    class CommandLineParser
    {
    private:
        std::vector<std::string> m_needles;
        std::string m_path {""};
        std::string m_errorString {""};
        bool m_isValid {false};
        const std::string STR_SAMPLE_USAGE {"Sample usage: file-finder.exe path <substring1> [<substring2> [<substring3>] ...]"};
        const std::string STR_PLEASE_SPECIFY {"Please specify both a path and at least one substring to search for."};

        ///  Handles parsing of command line arguments and sets object properties accordingly.
        void ParseCommandLine(int argc, char *argv[]);
    public:

        CommandLineParser() = delete;

        CommandLineParser(int argc, char *argv[]);

        /// Returns true if command line was successfully parsed, otherwise false 
        bool IsValid()  const;

        /// If command line parsed successfully returns the path parameter, otherwise returns an empty string
        std::string Path() const;

        /// Returns the number of substrings specified on the command line that we will use to find the, "needles" in our haystacks 
        std::vector<std::string> Needles() const;

        /// If Parse has returned false, will contain error string that can be dipslayed to user
        std::string ErrorString() const;
    };
}

