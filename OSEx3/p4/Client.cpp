#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>


using namespace std;

string getresponce(int sock) {
    char buffer[1024] = {0};
    ssize_t valread = read(sock, buffer, 1024);
    if (valread < 0) {
        cerr << "Read failed" << endl;
        return "";
    }
    buffer[valread] = '\0';
    return string(buffer);

}

void sendCommand(int sock, string command) {
    //send the commend if comend is not empty
    if (command.empty()){
    send(sock, command.c_str(), command.size(), 0);
    cout << getresponce(sock) << endl;
    }

}

int main() {
    cout << "Welcome to the client!" << endl;
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // Create a socket for the client
    cout << "Creating socket..." << endl;
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        cerr << "Socket creation failed" << endl;
        return 1;
    }

    //set the server address details
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9037);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        cerr << "Invalid address" << endl;
        return 1;
    }

    cout << "Connecting to server..." << endl;
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Connection failed" << endl;
        return 1;
    }


    //recive and print the welcome message from the server
    cout << getresponce(sock) << endl;

    //start the communication
    cout << "Starting communication..." << endl;
    string input;
    while (getline(cin, input)) {
        sendCommand(sock, input + "\n");

        if (input == "Exit") {
            break;
        }
        if (input == "Kosaraju") {
            cout << "Waiting for the server to compute SCCs..." << endl;
            cout << getresponce(sock) << endl;
        }
        if (input.rfind("Newgraph", 0) == 0)
        {
            istringstream iss(input);
            string command;
            int n, m;
            if (!(iss >> command >> n >> m))
            {
                cout << "Invalid command format" << endl;
                continue;
            }
            for (int i = 0; i < m; ++i)
            {
                if (!getline(cin, input))
                {
                    cout << "Input error" << endl;
                    close(sock);
                    return 1;
                }
                sendCommand(sock, input + "\n");
            }
        }

        cout << getresponce(sock) << endl;
    }

    close(sock);
    return 0;
}
