#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <getopt.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/socket.h>


// Buffer size for communication
#define BUFFER_SIZE 1024


// Function to execute a program with specific I/O redirection based on mode
void dupandexec(pid_t pid, char mode, int client_sock, int server_sock, const char *exe, const char *arg){
    printf("in dupandexec\n");
    //explanation to this part:
    // we dont have to change the input cecause the input is already coming from the stdin (to the socket)
    // we have to change the output to the socket because we wnat to despaly the output to the client
    if (pid == 0) { // Child process
        switch (mode) {
            case 'i':
                printf("case i\n");
                // print the exe
                printf("exe: %s\n", exe);                                                               //debug print delete later
                dup2(client_sock, 1);  // Redirect standard output to the client socket
                printf("dup2 done\n");                                                                  //debug print delete later
                break;

            case 'b':
            printf("case b\n");
                dup2(client_sock, 0);  // Redirect both input and output
                dup2(client_sock, 1);
                break;
        }
        // Execute the specified program
        printf("Executing command: %s\n", exe);
        // Prepare the argument vector
        char *args[] = { (char *)exe, (char *)arg, NULL };  // Argument vector
        // Execute the command using execvp
        execvp(exe, args);
        // If execvp returns, there was an error
        perror("Execution of exe failed");
        exit(EXIT_FAILURE);
    }
    
    // Wait for the child process to finish
    int status;
    waitpid(pid, &status, 0);

    // Close the sockets based on whether client_sock and server_sock are the same
    if (client_sock == server_sock) {
        close(client_sock);
    } else {
        close(server_sock);
        close(client_sock);
    }
}

// Function to handle communication over a TCP connection
void TCPchat(int client_sock, pid_t pid, char *buffer){
    printf("Welcome to TCP chat\n");
    if (pid == 0) { // Child process: read from server
        while (1) {
            // Read data from the client
            ssize_t bytes_read = read(client_sock, buffer, BUFFER_SIZE - 1);
            if (bytes_read <= 0) {
                break; // Exit loop if no data or error occurs
            }
            buffer[bytes_read] = '\0'; // Null-terminate the string
            printf("Received from server: %s\n", buffer); // Print received data
        }
        close(client_sock); // Close the socket
        exit(0); // Exit child process
    } else { // Parent process: write to server
        while (1) {
            printf("Enter message (-1 to quit): \n");
            // Get user input
            fgets(buffer, sizeof(buffer), stdin);
            //check for fgets error
            if(ferror(stdin)){
                perror("fgets failed");
                exit(EXIT_FAILURE);
            }
            // Remove newline character
            buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character

            // Exit loop if user enters "-1"
            if (strcmp(buffer, "-1") == 0) {
                break;
            }

            // Send user input to the server (check if the buffer is not empty)
            if (strlen(buffer) > 0){
                if (write(client_sock, buffer, strlen(buffer)) <= 0) {
                    break;
                }   
            }
        }
        // Close the socket and kill the child process that is reading from the server
        printf("Closing client socket and kiling the proces\n");
        close(client_sock); 
        kill(pid, SIGKILL);
    }
}



// Function to create a TCP client and initiate communication
int TCPclient(int port, char mode, const char *server_ip){
    printf("Creating TCP client\n");
    int client_sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Create a TCP socket
    printf("Creating TCP socket\n");
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set server address and port
    printf("Setting server address and port\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    // Connect to the server
    printf("Connecting to server\n");
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    // Create a child process to handle communication
    printf("Forking process to handle communication\n");
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    // Call the TCPchat function to handle TCP communication
    printf("Calling TCPchat function\n");
    TCPchat(client_sock, pid, buffer);
    return 0;
}

// Function to create a TCP server and handle client connections
int TCPserver(char mode, int port, const char *exe, const char *arg, int flag){
    printf("Creating TCP server\n");
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];

    // Create a TCP socket
    printf("Creating TCP socket\n");
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to allow reuse of the address
    printf("Setting socket options\n");
    int optval = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // Set server address and port
    printf("Setting server address and port\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the server socket
    printf("Binding server socket\n");
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    printf("Listening for incoming connections\n");
    if (listen(server_sock, 5) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    // Accept a client connection
    printf("Accepting client connection\n");
    socklen_t client_addr_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);
    if (client_sock < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    // Ensure the correct executable is specified
    printf("Verifying executable is the correct one\n");
    if(!flag){
        if (strcmp(exe, "./ttt") != 0) {
            puts(exe);
            perror("exe is not ./ttt");
            exit(EXIT_FAILURE);
        }
    }

    // Fork a child process to handle the client
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    // Handle communication or run a program based on the flag
    printf("choosing to chat or run by the flag\n");
    if (flag) {
        TCPchat(client_sock, pid, buffer);
    } else {
        dupandexec(pid, mode, client_sock, server_sock, exe, arg);
    }
    return 0;
}

// Function to handle communication over a UDP connection
void UDPchat(int client_sock, pid_t pid, char *buffer, struct sockaddr_in server_addr){
    printf("Welcome to UDP chat\n");
    if (pid == 0) { // Child process: read from server
        while (1) {
            // Read data from the client using recvfrom (UDP)
            ssize_t bytes_read = recvfrom(client_sock, buffer, BUFFER_SIZE, 0, NULL, 0);
            if (bytes_read <= 0) {
                break; // Exit loop if no data or error occurs
            }
            buffer[bytes_read] = '\0'; // Null-terminate the string
            printf("Received from server: %s\n", buffer); // Print received data
        }
        close(client_sock); // Close the socket
        exit(0); // Exit child process
    } else { // Parent process: write to server
        while (1) {
            printf("Enter message (-1 to quit): \n");
            // Get user input
            fgets(buffer, sizeof(buffer), stdin);
            // Remove newline character
            buffer[strcspn(buffer, "\n")] = '\0';

            // Exit loop if user enters "-1"
            if (strcmp(buffer, "-1") == 0) {
                break;
            }
            // Send user input to the server using sendto (UDP)
            if (sendto(client_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) <= 0) {
                // break;
            }
        }
        // Close the socket and kill the child process that is reading from the server
        printf("Closing client socket and kiling the proces\n");
        close(client_sock); // Close the socket
        kill(pid, SIGKILL);
    }
}

// Function to create a UDP server and handle communication
int UDPserver(char mode, int port, const char *exe, const char *arg, int flag, int timeout){
    printf("Creating UDP server\n");
    int server_sock;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];

    // Create a UDP socket
    printf("Creating UDP socket\n");
    server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to allow reuse of the address
    printf("Setting socket options\n");
    int optval = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // Set server address and port
    printf("Setting server address and port\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the server socket
    printf("Binding server socket\n");
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Ensure the correct executable is specified
    printf("Verifying executable is the correct one\n");
    if(!flag){
        if (strcmp(exe, "./ttt") != 0) {
            puts(exe);
            perror("exe is not ./ttt");
            exit(EXIT_FAILURE);
        }
    }

    // // Receive data from the client
    printf("Receiving data from the client\n");
    socklen_t client_addr_size = sizeof(client_addr);
    memset(&client_addr, 0, client_addr_size);
    size_t bytes = recvfrom(server_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_size);
    printf("Received from client: %s\n", buffer);

    // Fork a child process to handle the client
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    // Set a timeout if specified
    if (timeout > 0) {
        printf("Setting timeout for %d seconds\n", timeout);
        alarm(timeout);
    }

    // Handle communication or run a program based on the run flag
    printf("choosing to chat or run by the flag\n");
    if (!flag) {
        dupandexec(pid, mode, server_sock, server_sock, exe, arg);
    }else if(flag && pid == 0){
        UDPchat(server_sock, pid, buffer, client_addr);
    }
     else {
        UDPchat(server_sock, pid, buffer, client_addr);
        }
    return 0;
}

// Function to create a UDP client and initiate communication
int UDPclient(int port, char mode, const char *server_ip, int timeout){
    printf("Creating UDP client\n");
    int client_sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Create a UDP socket
    printf("Creating UDP socket\n");
    client_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set server address and port
    printf("Setting server address and port\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    // Send data to the server
    printf("Sending data to the server\n");
    sendto(client_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // Fork a child process to handle communication
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    // Set a timeout if specified
    if (timeout > 0) {
        printf("Setting timeout for %d seconds\n", timeout);
        alarm(timeout);
    }

    // Handle UDP communication
    memset(buffer, 0, BUFFER_SIZE);
    UDPchat(client_sock, pid, buffer, server_addr);
    return 0;
}

// Function to handle communication over a UNIX Datagram socket
void UDSchat(int client_sock, pid_t pid, char* buffer, struct sockaddr_un server_addr) {
    if (pid == 0) { // Child process: read from server
        while (1) {
            // Read data from the client using recvfrom (UDP-style communication)
            ssize_t bytes_read = recvfrom(client_sock, buffer, BUFFER_SIZE, 0, NULL, 0);
            if (bytes_read <= 0) {
                perror("read failed");  // If reading fails or connection is closed, break the loop
                break;
            }
            buffer[bytes_read] = '\0';  // Null-terminate the string
            printf("Received from server: %s\n", buffer);  // Print the data received from server
        }
        close(client_sock);  // Close the client socket after done reading
        exit(0);  // Exit child process
    } else {  // Parent process: write to server
        while (1) {
            printf("Enter message (-1 to quit): \n");
            fgets(buffer, sizeof(buffer), stdin);  // Get user input
            buffer[strcspn(buffer, "\n")] = '\0';  // Remove newline character
            if (strcmp(buffer, "-1") == 0) {
                break;  // Exit loop if user enters "-1"
            }
            printf("%s\n", server_addr.sun_path);  // Print server path (for debug purposes)
            
            // Send user input to the server using sendto (Datagram communication)
            if (sendto(client_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) <= 0) {
                perror("sendto failed");  // If sendto fails, print an error and exit
                break;
            }
        }
        close(client_sock);  // Close client socket when done writing
        kill(pid, SIGKILL);  // Kill the child process that was reading from the server
    }
}

// Function to create a UNIX Domain Socket Datagram (SOCK_DGRAM) server
int UDSDatagramServer(char mode, char *exe, const char* path, const char *arg, int run, int timeout) {
    int server_sock;  // Server socket file descriptor
    struct sockaddr_un server_addr, client_addr;  // Structures for server and client addresses
    char buffer[BUFFER_SIZE];  // Buffer for communication

    // Check if the executable is "ttt", if not, print error and exit
    if (strcmp(exe, "./ttt") != 0) {
        puts(exe);
        perror("exe is not ttt sadge");
        exit(EXIT_FAILURE);
    }

    // Create a UNIX Datagram socket (SOCK_DGRAM)
    server_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (server_sock < 0) {
        perror("socket failed");  // Print error if socket creation fails
        exit(EXIT_FAILURE);
    }

    int optval = 1;  // This can be used if you want to set socket options

    // Set the server address family to UNIX domain and assign the file path
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, path);  // Copy the file path to the address structure

    // Unlink (remove) any previous socket file with the same name to avoid conflicts
    unlink(server_addr.sun_path);

    // Bind the server socket to the specified address and path
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");  // Print error if bind fails
        exit(EXIT_FAILURE);
    }

    // Set the length of the client address structure for receiving messages
    socklen_t client_addr_len = sizeof(client_addr);

    // Wait for a message from the client (blocking call)
    size_t bytes = recvfrom(server_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_addr_len);
    printf("Received from client: %s\n", buffer);  // Print the message received from the client

    // Print the client's socket path for debugging
    printf("lo nahon %s\n", client_addr.sun_path);

    // Fork a new process to handle communication with the client
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");  // Print error if fork fails
        exit(EXIT_FAILURE);
    }

    // If a timeout is specified, set an alarm to handle the timeout
    if (timeout > 0) {
        alarm(timeout);  // Set the timeout
    }

    // Based on the 'run' flag, either handle the communication or execute the program
    if (run) {
        // Run communication using UNIX datagram sockets (Datagram chat function)
        UDSchat(server_sock, pid, buffer, client_addr);
    } else {
        // Run the specified program using the child process
        dupandexec(pid, mode, server_sock, server_sock, exe, arg);
    }
    return 0;  // Return success
}


// Function to create a UNIX datagram socket client
int UDSDatagramClient(char mode, const char *path, int timeout) {
    int client_sock;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];
    strcpy(buffer, "barasi");  // Initial message to send to the server

    // Create UNIX datagram socket
    client_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (client_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, path);  // Copy the server path to the address structure

    // Send initial message to the server
    sendto(client_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // Fork a process to handle communication
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    
    // Set a timeout for the client (if specified)
    if (timeout > 0) {
        alarm(timeout);
    }

    // Clear the buffer and start the communication process
    memset(buffer, 0, BUFFER_SIZE);
    UDSchat(client_sock, pid, buffer, server_addr);  // Handle client-server communication
    return 0;
}

// Function to create a UNIX stream socket server
int UDSStreamServer(char mode, char *exe, const char *arg, int flag, const char *path) {   
    struct sockaddr_un server, client;
    memset(&server, 0, sizeof(server));  // Clear the server address structure
    memset(&client, 0, sizeof(client));  // Clear the client address structure
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, path);  // Copy the path to the server address structure

    int server_sock, client_sock;
    char buffer[BUFFER_SIZE];

    // Create UNIX stream socket (for connection-oriented communication)
    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Unlink any previous socket with the same path
    unlink(server.sun_path);

    // Bind the server socket to the specified path
    int len = strlen(server.sun_path) + sizeof(server.sun_family);
    printf("%s\n", server.sun_path);
    if (bind(server_sock, (struct sockaddr *)&server, len) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_sock, 1) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    // Accept an incoming connection
    socklen_t client_addr_size = sizeof(client);
    client_sock = accept(server_sock, (struct sockaddr *)&client, &client_addr_size);
    if (client_sock < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    // Fork a process to handle the client
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    // Handle communication or execute a program depending on the flag
    if (flag) {
        TCPchat(client_sock, pid, buffer);  // Handle stream communication
    } else {
        dupandexec(pid, mode, client_sock, server_sock, exe, arg);  // Run the specified program
    }
    return 0;
}

// Function to create a UNIX stream socket client
int UDSStreamClient(char mode, const char *path) {

    int client_sock;
    struct sockaddr_un server_addr = {.sun_family = AF_UNIX};
    char buffer[BUFFER_SIZE];

    // Create UNIX stream socket
    printf("Creating UNIX stream socket\n");
    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set the server address
    printf("Setting server address\n");
    strcpy(server_addr.sun_path, path);

    // Connect to the server
    printf("Connecting to server\n");
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    // Fork a process to handle communication
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    // Handle communication with the server
    TCPchat(client_sock, pid, buffer);
    return 0;
}


// Main function to handle command-line arguments and initiate server/client creation
int main(int argc, char *argv[])
{
    //Exemple comends to run the program for every part of the program
    // the formats of the arguments are as follows:
    // PART 3
    // ./mync -e "./ttt 123456789" -i TCPS4055
    // ./mync -e "./ttt 123456789" -o TCPClocalhost,4055
    // ./mync -e "./ttt 123456789" -b TCPS4055
    // PART 3.5
    // ./mync -e -i TCPS4060 
    // ./mync -e -o TCPClocalhost,4060  
    // PART 4
    // ./mync -e "./ttt 123456789" -i UDPS4050 -t 10                                  //with timeout 
    // ./mync -e "./ttt 123456789" -o UDPClocalhost,4050 -t 10                        //with timeout
    // ./mync -e "./ttt 123456789" -b UDPS4050 -t 10                                  //with timeout
    // PART 6
    // ./mync -e "./ttt 123456789" -i UDSSD/tmp/sock -t 10                            //with timeout
    // ./mync -e "./ttt 123456789" -o UDSCD/tmp/sock -t 10                            //with timeout

    // ./mync -e "./ttt 123456789" -i UDSSS/tmp/sock -t 10                           
    // ./mync -e "./ttt 123456789" -o UDSCS/tmp/sock -t 10                           
                           
    // over all it can be with or without timeout and it can be udp or tcp
    int flag = 1;
    char mode = 0;
    int port = 0;
    char *exe = NULL;
    char *arg = NULL;
    int timeout = -1;

    int opt;
    // Parse command-line options
    while ((opt = getopt(argc, argv, "e:t:iob")) != -1) {
        switch (opt) {
case 'e': // Executable argument
            printf("contains -e\n");
            exe = strtok(optarg, " ");  // First part of optarg is the executable
            arg = strtok(NULL, " ");    // The rest is the argument
            flag = 0;
            break;
            case 'i': // Input mode
                printf("contains -i\n");
                mode = opt;
                break;
            case 'o': // Output mode
                printf("contains -o\n");
                mode = opt;
                break;
            case 'b': // Both input and output mode
                printf("contains -b\n");
                mode = opt;
                break;
            case 't': // Timeout argument
                printf("contains -t\n");
                timeout = atoi(optarg);
                printf("timeout: %d\n", timeout);
                if (timeout <= 0) {
                    fprintf(stderr, "Timeout must be a positive integer\n");
                    exit(EXIT_FAILURE);
                }
                break;
        }
    }

    // Process remaining command-line arguments
    char *junk;    // junk variable to get into the right part of the string
    for (int i = 0; i < argc; i++) {
        if (strstr(argv[i], "TCPS")) { // TCP Server
            printf("contains TCPS, creating TCP server\n");
            junk = strtok(argv[i], "S");
            port = atoi(strtok(NULL, "S"));
            printf("port: %d\n", port);
            TCPserver(mode, port, exe, arg, flag);
        }
        if (strstr(argv[i], "TCPC")) { // TCP Client
        printf("contains TCPC, creating TCP client\n");
            junk = strtok(argv[i], ",");
            port = atoi(strtok(NULL, ","));
            printf("port: %d\n", port);
            TCPclient(port, mode, "127.0.0.1");
        }
        if (strstr(argv[i], "UDPS")) { // UDP Server
            printf("contains UDPS, creating UDP server\n");
            junk = strtok(argv[i], "S");
            port = atoi(strtok(NULL, "S"));
            printf("port: %d\n", port);
            UDPserver(mode, port, exe, arg, flag, timeout);
        }
        if (strstr(argv[i], "UDPC")) { // UDP Client
            printf("contains UDPC, creating UDP client\n");
            junk = strtok(argv[i], ",");
            port = atoi(strtok(NULL, ","));
            printf("port: %d\n", port);
            UDPclient(port, mode, "127.0.0.1", timeout);
        }
        if(strstr(argv[i],"UDSSD")){   //UDS Datagram Server
            printf("contains UDSSD, creating UDS datagram server\n");
            char* path = strtok(argv[i],"UDSSD");
            path += 5;
            UDSDatagramServer(mode, exe, path, arg, flag, timeout);
        }
        if(strstr(argv[i],"UDSCD")){  //UDS Datagram Client
            printf("contains UDSCD, creating UDS datagram client\n");
            char* path = strtok(argv[i],"UDSCD");
            path += 5;
            UDSDatagramClient(mode, path, timeout);
        }
        if(strstr(argv[i],"UDSSS")){ //UDS Stream Server
            printf("contains UDSSS, creating UDS stream server\n");
            char *path = strtok(argv[i],"UDSSS");
            path += 5;
            printf("exe:   %s\n",exe);
            printf("path:   %s\n",path);
            UDSStreamServer(mode, exe, arg, flag, path);
        }
        if(strstr(argv[i],"UDSCS")){ //UDS Stream Client
            printf("contains UDSCS, creating UDS stream client\n");
            char *path = strtok(argv[i],"UDSCS");
            path += 5;
            printf("%s\n",path);
            UDSStreamClient(mode, path);
        }
    }


    return 0;
}
