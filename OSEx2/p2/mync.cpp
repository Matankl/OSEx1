#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <sstream>  // Include this for std::istringstream
#include <iterator>
#include <cerrno>  // Include this for errno
#include <cstdio>  // Include this for perror()

int main(int argc, char *argv[])
{
    int opt;
    char **execArgs; // Declare execArgs properly

    // Initialize getopt in a while loop to process all arguments
    while ((opt = getopt(argc, argv, "e:")) != -1) {
        if (opt == 'e') {
            std::cout << "flag \"e\" found: " << optarg << std::endl;

            // Create string stream from optarg
            std::istringstream iss(optarg);
            // Tokenize by spaces
            std::vector<std::string> tokens{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};

            // Allocate memory for execArgs
            execArgs = new char *[tokens.size() + 1];                                   // Use char* for compatibility with exec
            for (size_t i = 0; i < tokens.size(); ++i)
            {
                execArgs[i] = strdup(tokens[i].c_str());                                // Use strdup to duplicate the string
            }
            execArgs[tokens.size()] = nullptr;                                          // Null-terminate the array

            // Example: Execute the command if required
            std::cout << "Executing command: " << execArgs[0] << std::endl;
            if (execvp(execArgs[0], execArgs) == -1) {                                  // Check for errors
                std::cerr << "Execution of " << execArgs[0] << " failed" << std::endl;
                perror("execvp");                                                       // Print error message
            }

            // Free allocated memory (if execvp returns)
            for (size_t i = 0; i < tokens.size(); ++i)
            {
                free(execArgs[i]);                                                      // Free duplicated strings
            }
            delete[] execArgs;                                                          // Free the array of pointers

        } else {
            std::cout << "Unknown flag: " << opt << std::endl;
        }
    }

    return 0;
}
