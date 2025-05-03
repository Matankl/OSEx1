#include <iostream>
#include <thread>
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <mutex>
#include <algorithm>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
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

// Function to handle client requests
void handleClient(int clientSocket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0) {
            cout << "Client disconnected.\n";
            close(clientSocket);
            break;
        }

        string command(buffer);
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
                bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
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

        write(clientSocket, response.c_str(), response.size());
    }
}

// Main function to start the server
int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << "Failed to create socket.\n";
        return 1;
    }

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

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            cerr << "Failed to accept client connection.\n";
            continue;
        }

        cout << "Client connected.\n";
        thread clientThread(handleClient, clientSocket);
        clientThread.detach();
    }

    close(serverSocket);
    return 0;
}
