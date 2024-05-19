#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstring>

int main(int argc, char *argv[]) {
    if (argc > 3) {
        std::cerr << "Usage: " << argv[0] << " <name>" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Create the first pipe
    int pipe1[2];
    if (pipe(pipe1) == -1) {
        perror("pipe1");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork1");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0) { // Child process for grep
        close(pipe1[0]); // Close unused read end
        dup2(pipe1[1], STDOUT_FILENO); // Redirect stdout to pipe write end
        close(pipe1[1]); // Close original write end

        execlp("grep", "grep", argv[1], "phonebook.txt", NULL);
        perror("execlp grep");
        exit(EXIT_FAILURE);
    } else { // Parent process
        close(pipe1[1]); // Close unused write end

        // Create the second pipe
        int pipe2[2];
        if (pipe(pipe2) == -1) {
            perror("pipe2");
            exit(EXIT_FAILURE);
        }

        pid_t pid2 = fork();
        if (pid2 == -1) {
            perror("fork2");
            exit(EXIT_FAILURE);
        }

        if (pid2 == 0) { // Child process for sed
            close(pipe2[0]); // Close unused read end
            dup2(pipe1[0], STDIN_FILENO); // Redirect stdin to read end of first pipe
            dup2(pipe2[1], STDOUT_FILENO); // Redirect stdout to write end of second pipe
            close(pipe1[0]); // Close original read end
            close(pipe2[1]); // Close original write end

            execlp("sed", "sed", "s/, / /g", NULL);
            perror("execlp sed");
            exit(EXIT_FAILURE);
        } else { // Parent process
            close(pipe1[0]); // Close read end of first pipe
            close(pipe2[1]); // Close write end of second pipe

            // Create the third pipe
            int pipe3[2];
            if (pipe(pipe3) == -1) {
                perror("pipe3");
                exit(EXIT_FAILURE);
            }

            pid_t pid3 = fork();
            if (pid3 == -1) {
                perror("fork3");
                exit(EXIT_FAILURE);
            }

            if (pid3 == 0) { // Child process for awk
                close(pipe3[0]); // Close unused read end
                dup2(pipe2[0], STDIN_FILENO); // Redirect stdin to read end of second pipe
                dup2(pipe3[1], STDOUT_FILENO); // Redirect stdout to write end of third pipe
                close(pipe2[0]); // Close original read end
                close(pipe3[1]); // Close original write end

                execlp("awk", "awk", "{print $2}", NULL);
                perror("execlp awk");
                exit(EXIT_FAILURE);
            } else { // Parent process
                close(pipe2[0]); // Close read end of second pipe
                close(pipe3[1]); // Close write end of third pipe

                // Wait for the third child process to finish and read output
                waitpid(pid3, NULL, 0);

                // Read the output from the third pipe
                char buffer[256];
                ssize_t bytesRead = read(pipe3[0], buffer, sizeof(buffer) - 1);
                if (bytesRead > 0) {
                    buffer[bytesRead] = '\0'; // Null-terminate the string
                    std::cout << buffer;
                }

                close(pipe3[0]); // Close read end of third pipe

                // Wait for the other child processes to finish
                waitpid(pid2, NULL, 0);
                waitpid(pid1, NULL, 0);
            }
        }
    }

    return 0;
}
