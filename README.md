# The File-Finder Project
The below document covers the file-finder project, including platform, usage, high level design decisions, test cases, and a list changes that would make the project better.

## Table of Contents
1. [Intention](#Intention)</br>
2. [Audience](#Audience)</br>
3. [About the project](#About-the-project)</br>
4. [Use case diagram and requirements](#Use-case-diagram-and-requirements)</br>
5. [Technical Use Cases](#Technical-Use-Cases)</br>
6. [Domain Analysis](#Domain-Analysis)</br>
7. [Tests I would have written](#Tests-I-would-have-written)</br>
8. [Things I would like to do if I had more time](#Things-I-would-like-to-do-if-I-had-more-time)
9. [Methodology](#Methodology)</br>

## Intention
The documentation below is designed to communicate about the thought exercise that lead to design decisions in the creation of file-finder, the changes that could be made to imrpove the project in the future, and to detail the methodology used.

## Audience
The documentation below is designed for a relatively technical audience familiar with the structure of use cases, use case diagrams, and class diagrams; though an effort has been made to keep this information as accessible as possible. The use case format implemented in this document is based on templates described in, "Patterns for writing effective use cases" by Alistair Cockburn.

## About the project
File-finder is a utility application designed to allow command line users to search for multiple partial file name matches in a specified search path, and obtain a list of matching files from the console. The program will dump results to the console every 5 seconds, unless a user hits any key to dump the results sooner, or presses 'q' to dump their final results and quit.

### Platform
This project has been implemented for Windows using Visual Studio 2017 Professional with C++, using the ISO C++17 Standard.

### Sample usage
`Sample usage: file-finder.exe path <substring1> [<substring2> [<substring3>] ...]`

## Use case diagram and requirements
Below is listed a use case diagram for the project that describes how the software will be used, and links those cases with the requirements of the software (e.g. what you'd list as it's features on a website).

![Use case diagram and requirements](https://greendoorgames.com/UCD.png)

### Technical Use Cases
The use cases below are used to think through the problem instead of just jumping right into code, so that runtime behavior is thought through. This exercise acts as a first implementation that can lead to more a robust implementation without the need to refactor code and unit tests on the first iteration.

#### CLI User Executes program with command line arguments

##### Main Path
1. This use case begins when a CLI User calls file-finder from the command line, specifying a directory as the first space delimited argument, and then 1..n string delimited arguments after that.
2. The application parses the command line arguments into a string representing the folder to be searched, and a list of string patterns (needles) to be matched against existing folders (haystacks).
3. The application verifies that the folder name specified exists and finds that it does.
4. The application creates an object that will buffer directory data in a thread safe manner.
5. The application creates an object that will manage the lifetime of search threads in the application, as well as monitor the search results as buffer data is processed for each substring and periodically dump that data to the console (unless a user requests an exit).
5. The application creates 1..n objects for each substring that will be responsible for searching buffered directory and subdirectory data given the specified path
6. The application begins buffering and searching operations
  6.a. [see: Application progressively buffers filesystem data for multiple consumers](#Application parses-recursive-directory-information-into-buffer).
  6.b. [see: Application executes search operation for substring](#Application-executes-search-operation-for-substring)]
7. The application waits on user input for 5 seconds, or for all search threads to complete
  7.a. If the CLIUser requests an exit by typing, 'quit' and pressing 'Enter', the application requests that each thread terminates, and cleans up all allocated data.
  7.b. If the CLIUser requests a dump by typing 'dump' and pressing 'Enter' or 5 seconds has elapsed, the application will dump any data received in a callback during [see: Application executes search operation for substring](#Application-executes-search-operation-for-substring) and clean up all dumped data from the container.
5. This use case ends when any remaining results are dumped line-by-line to the command prompt, all resources are cleaned up, and the program exits.

##### Alternative Path(s)

(1a) This use case ends when no arguments are specified and the user receives a message indicating that no parameters were found, along with sample usage, "Error: No arguments specified. Sample usage: file-finder ...".

(1b) This use case ends when no substring arguments are specified and the user receives a message indicating that no parameters were found, along with sample usage, "Error: No search string(s) specified. Sample usage: file-finder...".

(1c) This use case ends when the target search folder is not found. An error is returned to the command line, "Error: Search folder specified not found. Sample usage: file-finder...".

#### Application progressively buffers filesystem data for multiple consumers

##### Main Path
1. This use case begins when the application has verified the user has entered a valid path and substrings and starts to iterate all of the file names in the specified directory recursively.
2. The application allocates 64 lists of 1024 strings and places them in a thread safe queue for writing, and tracks the total bufffer count as 64.
3. The application pops the next available list from the queue and recursively iterates through 1024 file names and places them in the buffer.
4. The application notifies the threads performing search operations [see: Application executes search operation for substring](#Application-executes-search-operation-for-substring) that there is a new read-only buffer for processing.
5. The application repeats steps 3-4 until all files names have been processed.
6. This use case ends when all file names have been buffered and moved into a queue for reading and the threads performing search operations in [see: Application executes search operation for substring](#Application-executes-search-operation-for-substring) and the number of empty write buffers has been restored to the total buffer amount (indicating that all processing is complete and that we can clean up).


##### Alternative Path(s)
(3a) This use case ends if there are less than 1024 file names remaining to iterate the application enqueues the buffer in a threads safe queue for reading and notifies the threads performing search operations in [see: Application executes search operation for substring](#Application-executes-search-operation-for-substring) that there is a new buffer for processing.

(3b) The application attempts to pop the next available list from the queue but there are no more lists available
(3b.1) The application creates a new list of 1024 items, and adds it to the write queue, as well as incrementing the total buffer count by 1.

(5a) When a thread notifies the application that it's finished processing a buffer, the application increments a counter tracking the number of times the list buffer has been processed
(5a.1) If the number of the buffer has been processed matches the number of threads, the application clears the list buffer and enqueues it back into the thread safe queue for writing to prevent additional unnecessary buffer allocations

#### Application executes search operation for substring

##### Main Path
1. This use case begins when the application has notified the threads searching for substrings that a list buffer is ready to process in [see: Application progressively buffers filesystem data for multiple consumers](#Application parses-recursive-directory-information-into-buffer).
2. The search operation thread iterates over each file name found in the buffer.
3. For each set of files in the buffer the search operation thread will use std::boyer_moyer to see if it can find the, "needle" in the file name haystack.
4. If a match is found, the search operation thread triggers a callback in the main function to add a list of the found files for that directory.
5. Once the search operation completes for this list buffer the thread notifies the buffer object that it's done processing the buffer
6. This use case ends when all buffers for the specified path have been iterated through and there is no more work to be done, *or* if the parent object indicates it wishes to stop searching, at which time the thread cleans up any data and terminates.

##### Alternative Path(s)
(2a) If a search operation is already running the application enqueues the buffer in a thread safe queue for later processing once the current buffer has been searched successfully.

(4a) If a match is not found, the search operation thread ignores the item and continues to iterate through the rest of the file names in the buffer.

### Domain Analysis
Below is a class diagram based on the nouns and verbs form the use case cases above, which creates a class structure based up the language used.

![Use case diagram and requriements](https://greendoorgames.com/Class.png)

### Tests cases to written
- CommandLineParser::IsValid - Test return value in the following scenarios:
  - Ensure that only one command line argument returns false
  -  Ensure that only two command line arguments returns false
  - Ensure that three command line arguments with a file name instead of path returns false
  - Ensure that three command line arguments with a directory that does not exist returns false
- CommandLineParser::Path - Test that path passed to ctor is returned
- CommandLineParser::Needles - Test that all needles passed in command line are returned
- CommandLineParser::ErrorString - Test return values in the following scenarios:
  - Ensure that only one command line argument returns sample usage
  - Ensure that only two command line arguments returns sample usage
  - Ensure that three command line arguments with a file name instead of path returns error indicating this
  - Ensure that three command line arguments with a directory that does not exist returns error indicating this
- ThreadSafeQueue::Enqueue/ThreadSafeQueue::Dequeue/ThreadSafeQueue::Size - Test that in a multi-threaded scenario deadlocks are avoided and multiple items are successfully added and removed.
- FilesystemHaystack::FindNeedles -
  - Test that matching callback is triggered 1..n times for matching patterns
  - Test that matching callback is triggered 0 times if no matching pattern is found
  - Test that exit flag in callback triggers cessation of search operation
  - Test that finished callback is triggered in all scenarios
- ResultsMonitor::SearchFilesystem - This is another scenario where my overall structure would have needed to include a generic file system wrapper based on an interface to provide mock filesystem data at runtime to child objects, as well as a generic input wrapper based on an interface that could provide mock keyboard input at runtime. Standard output could also be redirected for this test.
  - Test to ensure that when SearchFilesystem was called with no matches that TotalMatches returns 0
  - Test to ensure that when SearchFilesystem was called with 1..n matches that TotalMatches returns the correct number
  - Test to ensure that when SearchFilesystem was called that appropriate data is output ~every 5 seconds
  - Test to ensure that when SearchFilesystem was called that appropriate data is output sooner than 5 seconds if keyboard input is received
  - Test to ensure that when SearchFilesystem was called that the method terminates when appropriate keyboard input is received
- FileNameBuffer::PopulateBuffers - Use mocked object to a generic file system wrapper based on an interface to send mock test data at runtime to PopulateBuffers.
    - Test to ensure the correct number of buffers are generated for a set of test filesystem
    - Test to ensure that the buffer filled callback is triggered successfully the right number of times
    - Test to ensure that TotalBuffersCreated expands as expected if total files increases faster than buffers can be processed.
    - Test to ensure Stop method terminates PopulateBuffers as expected
    - Test to ensure that AllFileNamesHaveBeenProcessed returns the correct value if all files have been processed in a test callback and EnqueueProcessedBuffer has been called for each of them.


### Things I would like to do if I had more time
1. I may have failed on the Simplicity evaluation criteria. My idea of simplicity in user mode development means using at least a couple of basic classes to make reading and mocking easier, employing basic principles without adding excessive complexity (e.g. if there were more variability I might been more diligent about SOLID, especially for mocking out interfaces/superclasses). It did occur to me that I should have just used plain old modular C to demonstrate driver-level-friendly simplicity.
2. There's a potential here to use a *lot* of threads for a large set of search patterns, these could be limited to a pool based on the number of cores on the machine.
3. I want to research whether there's a more efficient algorithm than std::boyer_moyer that would perform better within a single thread (just out of curiosity, are we gaining the benefits we hope for with multithreading?).
4. Exception handling - I'm not really enough here! How would I handle hardware/system errors, and what would be the most elegant way to handle thread and access exceptions.
5. Create a C implementation that behaves comparably with the C++ implementation, and compare.
6. Abstract directory iteration for mock objects and unit testing that doesn't require system integration.
7. Purely for fun (definitely not production), I would like to see if the single threaded solution could gather a threshold of file names and run all comparisons at once on a GPU for better performance (I've done similar lookups with malware signatures in the past - this would not be used as a viable example, just something I would do for laughs)
8. ResultsMonitor may violate SRP, hardware input/output should be abstracted
9. Support for translations of some kind might be beneficial for a wider target demographic.

## Methodology
### Purpose
The methodology for this document is based upon technical use case and domain analysis design, which is ideal for small systems or teams without a product owner (it is not intended to be a long lasting comprehensive design document, but more as a one time thought exercise used to mitigate risk). It was selected due it's simplicity, and the power this process has to identify architectural and implementation flaws that would require time consuming refactoring. The use case diagram, use cases, and class diagrams act as the first iteration on coding the project. The document is not intended for round tripping, and is merely being added here to demonstrate process.

### Steps
To better understand this document it's helpful to know what steps are involved in this methodology (at least the simplified version I list here):
1. Define what the software will do (requirements/features), and how it will be used (use cases) in a use case diagram.
2. Look for missing features or use cases by linking these elements in the diagram, if there are any that don't link clearly to one another, then a user case or feature should be added (relationships do not need to be 1:1, but relationships need to exist)
3. Break up these relationships into chunks or "systems", and prioritize them by the risk each system poses to the success of the project (in a project this small, there will really only be one system)
4. For the most important system of functionality, write all the use cases about how the software will work.
5. After the use cases have been written, perform, "Domain Analysis" where a class diagram is created based on the nouns and verbs used in the use cases that were written in step 4.
6. Take the completed class diagram, apply SOLID principles to it (and design patterns if you wish) and write tests and code based on that design *(Note: I skipped the tests based on the requirements for the project, but I do list them above)*
7. Apply everything that was learned from the steps so far and iterate on the design with the next system of functionality *(note this project was too small for more than one system)*
8. Once all systems have been designed, this document gets filed away since the actual implementation will be more informative.
