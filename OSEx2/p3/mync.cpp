#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <csignal>
#include <ctime>

#define BUFFER_SIZE 1024


// Function to create a client socket, connect to the server, and handle communication
int create_client(int port, char mode, const std::string& server_ip) {
    int client_sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Create a socket for the client
    std::cout << "Creating client socket..." << std::endl;
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);  // Exit if socket creation fails
    }

    // Set up the server address struct
    std::cout << "Setting up server address..." << std::endl;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);  // Convert port to network byte order
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());  // Convert IP to binary form

    // Connect to the server
    std::cout << "Connecting to server..." << std::endl;
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);  // Exit if connection fails
    }

    // Fork the process to handle reading and writing concurrently
    std::cout << "Forking process to handle communication...(Client)" << std::endl;
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);  // Exit if fork fails
    }

    if (pid == 0) { // Child process: read from server
        while (true) {
            ssize_t bytes_read = read(client_sock, buffer, BUFFER_SIZE - 1);
            if (bytes_read <= 0) {
                break; // Exit loop on read error or server disconnect
            }
            buffer[bytes_read] = '\0';  // Null-terminate the received string
            std::cout << buffer << std::endl;
        }
        close(client_sock);  // Close the socket
        exit(0);  // Exit child process
    } else { // Parent process: write to server
        while (true) {
            std::cout << "Enter message (-1 to quit): ";
            std::cin.getline(buffer, BUFFER_SIZE);  // Get input from user

            if (strcmp(buffer, "-1") == 0) {
                break; // Exit loop if user enters -1
            }
            //write only if the buffer is not empty
            if(strlen(buffer) > 0){
                std::cout << "Sending message to server..." << std::endl;
            write(client_sock, buffer, strlen(buffer));  // Send message to server
            }
        }
        close(client_sock);  // Close the socket

        kill(pid, SIGKILL);  // Kill child process reading from server
    }
    return 0;
}

// Function to create a server socket, accept a client connection, and execute a command
int create_server(char mode, int port, const std::string& exe, const std::string& arg) {
    std::cout << "in server function" << std::endl;
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;

    // Create a socket for the server
    std::cout << "Creating server socket..." << std::endl;
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);  // Exit if socket creation fails
    }

    // Set up the server address struct
    std::cout << "Setting up server address..." << std::endl;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);  // Convert port to network byte order
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP

    // Bind the socket to the specified port
    std::cout << "Binding server socket..." << std::endl;
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);  // Exit if binding fails
    }

    // Listen for incoming connections
    std::cout << "Listening for incoming connections..." << std::endl;
    if (listen(server_sock, 5) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);  // Exit if listening fails
    }

    // Accept a client connection
    std::cout << "Accepting client connection..." << std::endl;
    socklen_t client_addr_size = sizeof(client_addr);
    std::cout << "before accept" << std::endl;                                                              // debug print
    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);
    std::cout << "after accept" << std::endl;                                                               // debug print
    if (client_sock < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);  // Exit if accepting a connection fails
    }

    // Verify the executable name
    std::cout << "Verifying executable..." << std::endl;
    std::cout << "Executable: " << exe << std::endl;
    if (exe != "./ttt") {
        perror("Invalid executable");
        exit(EXIT_FAILURE);  // Exit if the executable name is invalid
    }

    std::string exe_path = "./ttt";  // Set the executable path

    // Fork the process to execute the command
    std::cout << "Forking process to execute command...(Server)" << std::endl;
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);  // Exit if fork fails
    }

    if (pid == 0) { // Child process: execute the command

        //explanation to this part:
        // we dont have to change the input cecause the input is already coming from the stdin (to the socket)
        // we have to change the output to the socket because we wnat to despaly the output to the client

        switch (mode) {
            case 'i':
                std::cout << "case i" << std::endl;
                dup2(client_sock, 1);  // Redirect standard output to the client socket
                break;
                
            case 'b':
                std::cout << "case b" << std::endl;
                //dup2(client_sock, 0);  // Redirect both standard input and output to the client socket
                dup2(client_sock, 1);
                break;
        }
        std::cout << "Executing command: " << exe_path << std::endl;
        char *args[] = { const_cast<char*>(exe_path.c_str()),  const_cast<char*>(arg.c_str()), NULL };  // Argument vector
        execvp(exe_path.c_str(), args);  // Execute the command
        // If execvp returns, there was an error
        perror("Execution of exe failed");
        exit(EXIT_FAILURE);

    }

    // Parent process: wait for the child process to finish
    int status;
    waitpid(pid, &status, 0);
    close(client_sock);  // Close the client socket
    close(server_sock);  // Close the server socketS
    return 0;
}

int main(int argc, char *argv[]) {
    // Check the number of arguments (shuld be 7 or 5)
    std::cout << "Number of arguments: " << argc << std::endl;
    if (argc != 7 && argc != 5) {
        std::cerr << argc << " arguments provided. Invalid number of arguments!" << std::endl;
        exit(EXIT_FAILURE);  // Exit if the number of arguments is incorrect
    }
    std::cout << "after argument before mode" << std::endl;                                     // debug print 



    // if its i o or b
    // format (./p3 -e "./ttt 123456789" -i TCPS4050)
    // format (./p3 -e "./ttt 123456789" -o TCPClocalhost,4050)
    // format (./p3 -e "./ttt 123456789" -b TCPS4050)
    // bool isServer = false;  // Flag to check if it's a server
    if (argc == 5) {
        char mode = argv[3][1];  // Access mode from the third argument
        char* token;  // Token contains the port
        //if mode is b fork and make server and client
        if (mode == 'b') {
            std::cout << "Mode: " << mode << std::endl;
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork failed");
                exit(EXIT_FAILURE);  // Exit if fork fails
            }
            if (pid == 0) { // Child process: create server
                            const char* delim = "S";
                token = strtok(argv[4], delim);
                std::cout << token << std::endl;                                                         // DELETE LATER: Debugging
                token = strtok(nullptr, delim);
                std::cout << token << std::endl;                                                         // DELETE LATER: Debugging
                // isServer = true;
                std::cout << "its a server, creating server..." << std::endl;
                int port = atoi(token);
                char* execarg = strtok(argv[2], " ");
                std::string exe = execarg;
                std::cout << exe << std::endl;                                                          // DELETE LATER: Debugging
                execarg = strtok(nullptr, " ");
                std::string arg = execarg;
                std::cout << arg << std::endl;                                                          // DELETE LATER: Debugging
                create_server(mode, port, exe, arg);  // Create server



            } else { // Parent process: create client
                const char* delim = "S";
                token = strtok(argv[4], delim);
                std::cout << token << std::endl;                                                         // DELETE LATER: Debugging
                token = strtok(nullptr, delim);
                std::cout << token << std::endl;                                                         // DELETE LATER: Debugging
                // isServer = true;
                std::cout << "its a server, creating server..." << std::endl;
                int port = atoi(token);
                create_client(port, mode, "127.0.0.1");  // Create client
                }
                
        }else{
        //Check if it's a server
        std::cout << "Checking if it's a server..." << std::endl;
        if (strchr(argv[4], 'S')) {
            const char* delim = "S";
            token = strtok(argv[4], delim);
            std::cout << token << std::endl;                                                         // DELETE LATER: Debugging
            token = strtok(nullptr, delim);
            std::cout << token << std::endl;                                                         // DELETE LATER: Debugging
            // isServer = true;
            std::cout << "its a server, creating server..." << std::endl;
            int port = atoi(token);
            char* execarg = strtok(argv[2], " ");
            std::string exe = execarg;
            std::cout << exe << std::endl;                                                          // DELETE LATER: Debugging
            execarg = strtok(nullptr, " ");
            std::string arg = execarg;
            std::cout << arg << std::endl;                                                          // DELETE LATER: Debugging
            create_server(mode, port, exe, arg);  // Create server
            
        } else {
            std::cout << "its a client, creating client..." << std::endl;
            const char* delim = "C";
            token = strtok(argv[4], ",");
            std::cout << token << std::endl;                                                         // DELETE LATER: Debugging
            token = strtok(nullptr, ",");
            std::cout << token << std::endl;                                                         // DELETE LATER: Debugging
            int port = atoi(token);
            std::cout << port << std::endl;
            std::cout << "Client" << std::endl;
            create_client(port, mode, "127.0.0.1");  // Create client
        }
        }
    }else{
        // if its in the format of (mync -e "ttt 123456789" -i TCPS4050 -o TCPClocalhost,4455)

    }

    return 0;
}
