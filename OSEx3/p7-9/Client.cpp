#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

void interactWithServer(int socketFD) {
    string command;
    char buffer[1024];

    while (true) {
        cout << "Enter command (or type 'exit' to quit): ";
        getline(cin, command);

        if (command == "exit") {
            cout << "Exiting client.\n";
            break;
        }

        // Send the main command to the server
        if (send(socketFD, command.c_str(), command.size(), 0) == -1) {
            cerr << "Error sending data to server.\n";
            break;
        }

        // If the command is "Newgraph", handle additional edge inputs
        if (command.rfind("Newgraph", 0) == 0) {
            stringstream ss(command);
            string operation;
            int vertices, edges;
            ss >> operation >> vertices >> edges;

            cout << "Graph initialized. Now enter " << edges << " edges as pairs (u v):\n";

            for (int i = 0; i < edges; ++i) {
                string edgeCommand;
                cout << "Edge " << i + 1 << ": ";
                getline(cin, edgeCommand);

                // Send each edge to the server
                if (send(socketFD, edgeCommand.c_str(), edgeCommand.size(), 0) == -1) {
                    cerr << "Error sending edge data to server.\n";
                    break;
                }

                // Receive confirmation for the edge
                memset(buffer, 0, sizeof(buffer));
                int bytesReceived = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
                if (bytesReceived <= 0) {
                    cerr << "Error receiving response from server.\n";
                    break;
                }
                cout << "Server: " << buffer;
            }
        }

        // Receive response from the server for the main command
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived == -1) {
            cerr << "Error receiving data from server.\n";
            break;
        } else if (bytesReceived == 0) {
            cout << "Server closed the connection.\n";
            break;
        }

        // Display the server's response
        cout << "Server response:\n" << buffer << endl;
    }
}

int main() {
    // Server address and port
    const char* serverIP = "127.0.0.1";
    const int serverPort = 8080;

    // Create socket
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD == -1) {
        cerr << "Error creating socket.\n";
        return 1;
    }

    // Server address struct
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);

    // Convert server IP to binary form
    if (inet_pton(AF_INET, serverIP, &serverAddr.sin_addr) <= 0) {
        cerr << "Invalid address/Address not supported.\n";
        return 1;
    }

    // Connect to the server
    if (connect(socketFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        cerr << "Connection to the server failed.\n";
        close(socketFD);
        return 1;
    }

    cout << "Connected to the server.\n";
    interactWithServer(socketFD);

    // Close the socket
    close(socketFD);
    return 0;
}
