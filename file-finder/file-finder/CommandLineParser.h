#pragma once
#include <vector>
#include <string>
#include <filesystem>

namespace fileFinder
{
    /// <summary> CommandLineParser is a crude utility class for parsing the command line options and verifying that they are valid.
    /// <para>The object accepts command line arguments. <see cref="System::Console::WriteLine"/> for information about output statements.</para>
    /// </summary>
    class CommandLineParser
    {
    private:
        // Attributes
        std::vector<std::string> m_needles;
        std::string m_path {""};
        std::string m_errorString {""};
        bool m_isValid {false};
        const std::string STR_SAMPLE_USAGE {"Sample usage: file-finder.exe path <substring1> [<substring2> [<substring3>] ...]"};
        const std::string STR_PLEASE_SPECIFY {"Please specify both a path and at least one substring to search for."};

        /// <summary> Handles parsing of command line arguments and sets object properties accordingly.</summary>
        void ParseCommandLine(int argc, char *argv[]);
    public:

        CommandLineParser() = delete;

        CommandLineParser(int argc, char *argv[]);

        ///<summary> Returns true if command line was successfully parsed, otherwise false </summary>
        bool IsValid()  const;

        ///<summary> If command line parsed successfully returns the path parameter, otherwise returns an empty string</summary>
        std::string Path() const;

        ///<summary>Returns the number of substrings specified on the command line that we will use to find the, "needles" in our haystacks </summary>
        std::vector<std::string> Needles() const;

        ///<summary> If Parse has returned false, will contain error string that can be dipslayed to user</summary>
        std::string ErrorString() const;
    };
}

