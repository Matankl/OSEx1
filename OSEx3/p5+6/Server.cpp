#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <stack>

using namespace std;

class Graph {
    int n;  // Number of vertices
    vector<vector<int>> adj;  // Adjacency list for the original graph

public:
    Graph(int vertices = 0) : n(vertices) {
        adj.resize(n + 1);  // Resize adjacency list for 1-based indexing
    }

    void addEdge(int u, int v) {
        adj[u].push_back(v);
    }

    void removeEdge(int u, int v) {
        adj[u].erase(remove(adj[u].begin(), adj[u].end(), v), adj[u].end());
    }

    const vector<int>& getAdjList(int v) const {
        return adj[v];
    }

    int getNumVertices() const {
        return n;
    }

    Graph transposeGraph() const {
        Graph transposed(n);
        for (int u = 1; u <= n; ++u) {
            for (int v : adj[u]) {
                transposed.addEdge(v, u);
            }
        }
        return transposed;
    }

    void fillOrder(int v, vector<bool>& visited, stack<int>& Stack) const {
        visited[v] = true;
        for (int i : adj[v]) {
            if (!visited[i]) {
                fillOrder(i, visited, Stack);
            }
        }
        Stack.push(v);
    }

    void dfs(int v, vector<bool>& visited, vector<int>& component) const {
        visited[v] = true;
        component.push_back(v);
        for (int i : adj[v]) {
            if (!visited[i]) {
                dfs(i, visited, component);
            }
        }
    }

    string findSCCs() const {
        stack<int> Stack;
        vector<bool> visited(n + 1, false);
        for (int i = 1; i <= n; ++i) {
            if (!visited[i]) {
                fillOrder(i, visited, Stack);
            }
        }

        Graph transposed = transposeGraph();
        fill(visited.begin(), visited.end(), false);

        stringstream result;
        while (!Stack.empty()) {
            int v = Stack.top();
            Stack.pop();
            if (!visited[v]) {
                vector<int> component;
                transposed.dfs(v, visited, component);
                for (int vertex : component) {
                    result << vertex << " ";
                }
                result << "\n";
            }
        }
        return result.str();
    }
};

// Global graph object
Graph graph;

// Set a socket to non-blocking mode
void setNonBlocking(int socketFD) {
    int flags = fcntl(socketFD, F_GETFL, 0);
    if (flags == -1) {
        cerr << "Failed to get socket flags.\n";
        exit(EXIT_FAILURE);
    }
    if (fcntl(socketFD, F_SETFL, flags | O_NONBLOCK) == -1) {
        cerr << "Failed to set socket to non-blocking mode.\n";
        exit(EXIT_FAILURE);
    }
}

// Process client commands
string processCommand(const string& command, int clientSocket) {
    char buffer[1024];
    stringstream ss(command);
    string operation;
    ss >> operation;

    string response;

    if (operation == "Newgraph") {
        int vertices, edges;
        ss >> vertices >> edges;

        // Initialize the graph
        graph = Graph(vertices);
        response = "Graph initialized with " + to_string(vertices) + " vertices and " + to_string(edges) + " edges.\n";
        write(clientSocket, response.c_str(), response.size());

        // Receive `m` pairs of edges
        for (int i = 0; i < edges; ++i) {
            memset(buffer, 0, sizeof(buffer));
            int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
            if (bytesRead <= 0) {
                cerr << "Client disconnected during edge input.\n";
                break;
            }

            string edgeInput(buffer);
            stringstream edgeSS(edgeInput);
            int u, v;
            edgeSS >> u >> v;

            graph.addEdge(u, v);
            response = "Edge added: " + to_string(u) + " -> " + to_string(v) + "\n";
            write(clientSocket, response.c_str(), response.size());
            }

        response = "All edges added.\n";
    } else if (operation == "Newedge") {
        int u, v;
        ss >> u >> v;
        graph.addEdge(u, v);
        response = "Edge added: " + to_string(u) + " -> " + to_string(v) + "\n";
    } else if (operation == "Removeedge") {
        int u, v;
        ss >> u >> v;
        graph.removeEdge(u, v);
        response = "Edge removed: " + to_string(u) + " -> " + to_string(v) + "\n";
    } else if (operation == "Kosaraju") {
        response = "Strongly Connected Components:\n" + graph.findSCCs();
    } else {
        response = "Unknown command.\n";
    }

    return response;
}

// Main function to start the server with the Reactor pattern
int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << "Failed to create socket.\n";
        return 1;
    }

    setNonBlocking(serverSocket);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        cerr << "Failed to bind socket.\n";
        return 1;
    }

    if (listen(serverSocket, 5) == -1) {
        cerr << "Failed to listen on socket.\n";
        return 1;
    }

    cout << "Server is running on port 8080...\n";

    // Set up select
    fd_set masterSet, readSet;
    FD_ZERO(&masterSet);
    FD_SET(serverSocket, &masterSet);
    int maxFD = serverSocket;

    unordered_map<int, string> clientBuffers;

    while (true) {
        readSet = masterSet;

        int activity = select(maxFD + 1, &readSet, nullptr, nullptr, nullptr);
        if (activity < 0) {
            cerr << "Select error.\n";
            break;
        }

        for (int i = 0; i <= maxFD; ++i) {
            if (FD_ISSET(i, &readSet)) {
                if (i == serverSocket) {
                    // New connection
                    int clientSocket = accept(serverSocket, nullptr, nullptr);
                    if (clientSocket != -1) {
                        setNonBlocking(clientSocket);
                        FD_SET(clientSocket, &masterSet);
                        maxFD = max(maxFD, clientSocket);
                        clientBuffers[clientSocket] = "";
                        cout << "New client connected: " << clientSocket << endl;
                    }
                } else {
                    // Existing client
                    char buffer[1024];
                    memset(buffer, 0, sizeof(buffer));
                    int bytesRead = read(i, buffer, sizeof(buffer) - 1);

                    if (bytesRead <= 0) {
                        // Client disconnected
                        cout << "Client disconnected: " << i << endl;
                        close(i);
                        FD_CLR(i, &masterSet);
                        clientBuffers.erase(i);
                    } else {
                        // Append data to buffer
                        clientBuffers[i] += string(buffer);
                        if (clientBuffers[i].find('\n') != string::npos) {
                            // Process the complete command
                            string command = clientBuffers[i];
                            clientBuffers[i] = "";

                            string response = processCommand(command, i);
                            write(i, response.c_str(), response.size());
                        }
                    }
                }
            }
        }
    }

    close(serverSocket);
    return 0;
}
