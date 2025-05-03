//this class is used to create a chat between 2 endpoints (client and server from 2 diffrent terminals)
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
int create_client(int port, const std::string& server_ip) {
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
            std::cout << "Received from server: " << buffer << std::endl;
        }
        close(client_sock);  // Close the socket
        exit(0);  // Exit child process
    } else { // Parent process: write to server
        while (true) {
            std::cout << "Enter message (-1 to quit): \n";
            std::cin.getline(buffer, BUFFER_SIZE);  // Get input from user

            if (strcmp(buffer, "-1") == 0) {
                break; // Exit loop if user enters -1
            }

            write(client_sock, buffer, strlen(buffer));  // Send message to server
        }
        close(client_sock);  // Close the socket

        kill(pid, SIGKILL);  // Kill child process reading from server
    }
    return 0;
}

// Function to create a server socket, accept a client connection, and execute a command
int create_server(int port) {
    std::cout << "in server function" << std::endl;
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];

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
    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);
    if (client_sock < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);  // Exit if accepting a connection fails
    }

    // Fork the process to execute the command
    std::cout << "Forking process to execute command...(Server)" << std::endl;
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);  // Exit if fork fails
    }

    if (pid == 0) { // Child process: read from server
        while (true) {
            ssize_t bytes_read = read(client_sock, buffer, BUFFER_SIZE - 1);
            if (bytes_read <= 0) {
                break; // Exit loop on read error or client disconnect
            }
            buffer[bytes_read] = '\0';  // Null-terminate the received string
            std::cout << "Received from client: " << buffer << std::endl;
        }
        close(client_sock);  // Close the client socket
        exit(0);  // Exit child process
    } else { // Parent process: write to client
        while (true) {
            std::cout << "Enter message (-1 to quit): \n";
            std::cin.getline(buffer, BUFFER_SIZE);  // Get input from user

            if (strcmp(buffer, "-1") == 0) {
                break; // Exit loop if user enters -1
            }

            write(client_sock, buffer, strlen(buffer));  // Send message to client
        }
        close(client_sock);  // Close the client socket
    }
    return 0;
}




int main(int argc, char *argv[]) {
    // Check the number of arguments (shuld be 3)
    std::cout << "Number of arguments: " << argc << std::endl;
    if (argc != 3) {
        std::cerr << argc << " arguments provided. Invalid number of arguments!" << std::endl;
        exit(EXIT_FAILURE);  // Exit if the number of arguments is incorrect
    }


    // if its i or o
    // format server (./p3.5 -i TCPS4050)
    // format client  (./p3.5 -o TCPClocalhost,4050)
            char* token;  // Token contains the port
    bool isServer = false;  // Flag to check if it's a server
    if (argv[1][1] == 'i') {
        isServer = true;
        std::cout << "its a server, creating server..." << std::endl;
            const char* delim = "S";
            token = strtok(argv[2], delim);
            token = strtok(nullptr, delim);
            isServer = true;
            std::cout << "its a server, creating server..." << std::endl;
            int port = atoi(token);
            create_server(port);  // Create server


    } else { //its a client
        std::cout << "its a client, creating client..." << std::endl;
        const char* delim = "C";
        token = strtok(argv[2], ",");
        token = strtok(nullptr, ",");
        int port = atoi(token);
        std::cout << port << std::endl;
        create_client(port, "127.0.0.1");  // Create client
    }

    return 0;
}
