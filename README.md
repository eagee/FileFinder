# The File-Finder Project
File-finder is a utility application meant allow command line users to specify partial strings that can then be located and returned to the user in a useful format for other command line operations.

## Intention
This document is designed to communicate about the design decisions that have been made in the creation of file-finder and to provide details about the  thought exercise used to design this solution for interested engineers (E.g. The author  likes to think through chunks of a design before code is written the first time, in order to reduce refactoring time).

## Audience
This documentation is designed for a relatively technical audience familiar with the structure of use cases, use case diagrams, and class diagrams; though an effort has been made to keep this information as accessible as possible. The use case format implemented in this document is based on templates described in, "Patterns for writing effective use cases" by Alistair Cockburn.

## Methodology
### Purpose
The methodology for this document is based upon technical use case and domain analysis design, which is ideal for small systems or teams without a product owner (it is not intended to be a long lasting comprehensive design document, but more as a thought exercise). It was selected due it's simplicity, and the power this process has to identify architectural and implementation flaws that would require time consuming refactoring. The use case diagram, use cases, and class diagrams act as the first iteration on coding the project. The document is not intended for round tripping, and as merely being added here to demonstrate process.

### Steps [Simplified]
To better understand this document it's helpful to know what steps are involved in this methodology:
1. Define what the software will do (requirements/features), and how it will be used (use cases) in a use case diagram.
2. Look for missing features or use cases by linking these elements in the diagram, if there are any that don't link clearly to another, then a user case or feature should be added (relationships do not need to be 1:1)
3. Break up these relationships into chunks or "systems", and prioritize them by the risk each system poses to the success of the project (in a project this small, there will really only be one system)
4. For the most important system of functionality, write all the use cases about how the software will work.
5. After the use cases have been written, perform, "Domain Analysis" where a class diagram is created based on the nouns and verbs used in the use cases that were written in step 4.
6. Take the completed class diagram, apply SOLID principles to it (and design patterns if you wish) and write tests and code based onthat design *(Note: I skipped the tests based on the requirements for the project, but I do list them below)*
7. Apply everything that was learned from the steps so far and iterate on the design with the next system of functionality *(note this project was too small for more than one system)*
8. Once all systems have been designed, this document gets filed away in lieu of the actual implementation.

## Use case diagram and requirements
Below is listed a use case diagram for the project that describes how the software will be used, and links those cases with the requirements of the software (e.g. what you'd list as it's features on a website).

![Use case diagram and requriements](/UCD.png)

### Technical Use Cases
The use cases below are used to think through the problem instead of just jumping right into code, so that runtime behavior is thought through. This exercise acts as a first bad implementation that a more robust implementation can be based on, it is also a great document for test cases to be generated from.

#### CLI User Executes program with command line arguments

##### Main Path
1. This use case begins when a CLI User calls file-finder from the command line, specifying a directory as the first space delimited argument, and then 1..n string delimited arguments after that.
2. The application parses the command line arguments into a string representing the folder to be searched, and a list of string patterns (needles) to be matched against existing folders (haystacks).
3. The application verifies that the folder name specified exists and finds that it does.
4. The application creates an object that will manage the lifetime of all threads in the application, as well as monitor the search results found for each substring and periodically dump that data to the console (unless a user requests an exit).
5. The application creates 1..n objects for each substring that will be responsible for searching the specified directory and all subdirectories and triggers a search for each object [see: Application executes search operation for substring](#Application-executes-search-operation-for-substring)
6. The application waits on user input for 5 seconds, or for all search threads to complete
7. If the CLIUser requests an exit by pressing, 'q', the application requests that each thread terminates, and cleans up all allocated data
8. If the CLIUser requests a dump by pressing any key or 5 seconds has elapsed, the application will dump any data received in a callback during [see: Application executes search operation for substring](#Application-executes-search-operation-for-substring) and clean up all dumped data from the container.
5. This use case ends when any remaining results are dumped line-by-line to the command prompt, all resources are cleaned up, and the program exits.

##### Alternative Path(s)

(1a) This use case ends when no arguments are specified and the user receives a message indicating that no parameters were found, along with sample usage, "Error: No arguments specified. Sample usage: file-finder ...".

(1b) This use case ends when no substring arguments are specified and the user receives a message indicating that no parameters were found, along with sample usage, "Error: No search string(s) specified. Sample usage: file-finder...".

(1c) This use case ends when the target search folder is not found. An error is returned to the command line, "Error: Search folder specified not found. Sample usage: file-finder...".

#### Application executes search operation for substring

##### Main Path
1. This use case begins when the application passes a list of paths and a substring to the search operation thread
2. The search operation thread recursively obtains a list of files for each subdirectory included in the path
3. For each set of files in a folder the search operation thread will use std::boyer_moyer to see if it can find the, "needle" in the file name haystack
4. If a match is found, the search operation thread triggers a callback in the main function to add a list of the found files for that directory
5. This use case ends when all subdirectories for the specified path have been iterated through and there is no more work to be done, *or* if the parent object indicates it wishes to stop searching, at which time the thread cleans up any data and terminates

##### Alternative Path(s)
(4a) If a match is not found, the search operation thread ignores the item and continues to iterate through the rest of the files

### Domain Analysis
Below is a class diagram based on the nouns and verbs form the use case cases above, which creates a class structure based up the language used.

![Use case diagram and requriements](/Class.png)

##### Things I would have tested
1. Test that command line arguments can be mocked and verified


### Things I would like to do if I had more time
1. I may have failed on the Simplicity evaluation criteria. My idea of simplicity in user mode development means using at least a couple of basic classes to make reading and mocking easier, employing basic principles without adding excessive complexity (e.g. if there were more variability I might use the strategy pattern / always program to an interface not an implementation). I also focused on C++ 17 because the standard provides shared common language that can be understood regardless of platform.
2. Having ResultsMonitor::MonitorSearch handle user input feels like a violation of SRP, I'd like to have a better abstraction here.
3. I made an assumption to leave out Unicode support for this example to save on time, that would never fly in production. (I'm pretty sure a language like Japanese would cause failures).
4. I would add translation support once I added unicode support if I felt this tool had a broad target demographic.
5. I want to research whether there's a more efficient algorithm that would perform better in a single thread (just out of curiosity).
6. Exception handling - I'm not really doing much here! How would I handle hardware/system errors, and what would be the most elegant way to handle thread exceptions. What about an access denied exception if I tried to enumerate the Windows folder? (I know we have an assumption that we have access in the project)
7. Create a C implementation that behaves comparably with the C++ implementation, for fun and just to get back to my C roots :). Compare mentally the complexity of each solution and learn from it.
8. Abstract directory iteration for mock support and unit testing that doesn't require system integration.
9. Look into whether we would see better performance only iterating through the filesystem once instead of on each thread
10. toLower implementation in ResultsMonitor::MonitorSearch should probably exist in a convenience method or utility class
11. Purely for fun, I would like to see if the single threaded solution could gather a threshold of file names and run all comparisons at once on a GPU for better performance (I've done similar lookups with malware signature matching in the past and saw a 5x-6x improvement for a single threaded test - this would not be used as a viable example, just something I would do for laughs)
