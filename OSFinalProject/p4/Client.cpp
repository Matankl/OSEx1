/*******************************************************
 * client.cpp
 * A simple TCP client that:
 *   - Connects to the server on a given IP/port.
 *   - Reads commands from std::cin
 *   - Sends each command to the server.
 *   - Waits for the server's response and prints it.
 *******************************************************/

#include <iostream>
#include <string>
#include <cstring>      // for memset
#include <unistd.h>     // for close(), read(), write()
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main() {
    std::string serverIP   = "127.0.0.1"; 
    int         serverPort = 12345;

    // Create socket
    int sockFD = socket(AF_INET, SOCK_STREAM, 0);
    if(sockFD < 0) {
        std::cerr << "[Client] socket() failed\n";
        return 1;
    }

    // Connect
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port   = htons(serverPort);
    inet_pton(AF_INET, serverIP.c_str(), &servAddr.sin_addr);

    if(connect(sockFD, (sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        std::cerr << "[Client] connect() failed\n";
        close(sockFD);
        return 1;
    }

    std::cout << "[Client] Connected to server at " << serverIP << ":" << serverPort << std::endl;

    // Main loop: read user commands, send to server, read reply.
    while(true) {
        // Prompt
        std::cout << "Enter command (or QUIT/EXIT): ";
        std::string line;
        if(!std::getline(std::cin, line)) {
            std::cout << "[Client] End of input.\n";
            break;
        }

        if(line.empty()) {
            continue;
        }

        // Send the line to the server
        line.push_back('\n');  // ensure newline
        int bytesSent = write(sockFD, line.c_str(), line.size());
        if(bytesSent < 0) {
            std::cerr << "[Client] write() failed\n";
            break;
        }

        // If the user typed QUIT or EXIT, we expect the server to close soon
        // but let's still try to read the server's final response
        if(line.find("QUIT") == 0 || line.find("EXIT") == 0) {
            // read once and then break
            char buffer[1024];
            memset(buffer,0,1024);
            int n = read(sockFD, buffer, 1023);
            if(n > 0) {
                std::cout << "[Server Reply] " << buffer << std::endl;
            }
            std::cout << "[Client] Closing.\n";
            break;
        }

        // Read server response (one line). 
        char buffer[1024];
        memset(buffer,0,1024);
        int n = read(sockFD, buffer, 1023);
        if(n <= 0) {
            std::cerr << "[Client] Server closed connection or error.\n";
            break;
        }
        // Print server's reply
        std::cout << "[Server Reply] " << buffer << std::endl;
    }

    close(sockFD);
    return 0;
}
