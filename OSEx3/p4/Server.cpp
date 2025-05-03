#include <iostream>
#include <deque>
#include <vector>
#include <algorithm>
#include <sstream>
#include <thread>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>  // for memset

using namespace std;

// Graph class with adjacency list and operations to add/remove edges
class Graph {
    int n;  // Number of vertices
    vector<deque<int>> adj;  // Adjacency list using deque

public:
    // Constructor to initialize the graph with a given number of vertices
    Graph(int vertices = 0) : n(vertices) {
        adj.resize(n + 1);  // Resize adjacency list for 1-based indexing
    }

    // Function to add an edge from vertex 'u' to vertex 'v'
    void addEdge(int u, int v) {
        adj[u].push_back(v);  // Add edge u -> v in the original graph
    }

    // Function to remove an edge from vertex 'u' to vertex 'v'
    void removeEdge(int u, int v) {
        auto it = find(adj[u].begin(), adj[u].end(), v);
        if (it != adj[u].end()) {
            adj[u].erase(it);  // Erase the edge v from the adjacency list of u
        }
    }

    // Get the number of vertices in the graph
    int getNumVertices() const {
        return n;
    }

    // Get the adjacency list of a specific vertex
    const deque<int>& getAdjList(int v) const {
        return adj[v];
    }

    // Function to return a transposed graph (reverse all edges)
    Graph transposeGraph() const {
        Graph transposed(n);  // Create a new graph with the same number of vertices
        for (int u = 1; u <= n; ++u) {
            for (int v : adj[u]) {
                transposed.addEdge(v, u);  // Reverse the edge u -> v becomes v -> u
            }
        }
        return transposed;
    }
};

// Global mutex to ensure thread-safe graph operations
mutex graph_mutex;

// Helper function for DFS and filling the stack in order of completion time
void fillOrder(const Graph &g, int v, vector<bool> &visited, deque<int> &Stack) {
    visited[v] = true;
    for (int i : g.getAdjList(v)) {
        if (!visited[i]) {
            fillOrder(g, i, visited, Stack);  // Recursively visit all unvisited neighbors
        }
    }
    Stack.push_back(v);  // Push the current vertex to the stack after visiting all neighbors
}

// Helper function to perform DFS on the transposed graph and collect strongly connected components
void dfs(const Graph &g, int v, vector<bool> &visited, deque<int> &component) {
    visited[v] = true;
    component.push_back(v);  // Add the current vertex to the component
    for (int i : g.getAdjList(v)) {
        if (!visited[i]) {
            dfs(g, i, visited, component);  // Recursively visit all unvisited neighbors
        }
    }
}

// Function to print the strongly connected components (SCCs) using Kosaraju's algorithm
void printSCCs(const Graph &g) {
    int n = g.getNumVertices();
    deque<int> Stack;
    vector<bool> visited(n + 1, false);  // Initialize visited array

    // Perform DFS on the original graph and fill the stack
    for (int i = 1; i <= n; ++i) {
        if (!visited[i]) {
            fillOrder(g, i, visited, Stack);
        }
    }

    // Get the transposed graph (reversed edges)
    Graph transposed = g.transposeGraph();
    fill(visited.begin(), visited.end(), false);  // Reset the visited array

    // Process vertices in decreasing order of completion time (using the stack)
    while (!Stack.empty()) {
        int v = Stack.back();
        Stack.pop_back();

        if (!visited[v]) {
            deque<int> component;  // Store the current strongly connected component (SCC)
            dfs(transposed, v, visited, component);  // Start DFS from vertex 'v'

            // Print the current SCC
            for (int vertex : component) {
                cout << vertex << " ";
            }
            cout << endl;  // Newline after each SCC
        }
    }
}

// Function to handle client requests
void handleClient(int client_socket) {
    Graph g;  // Initialize an empty graph
    char buffer[1024] = {0};  // Buffer to read client commands

    // Keep handling client commands until the client sends "Exit"
    while (true) {
        ssize_t valread = read(client_socket, buffer, 1024);
        if (valread < 0) {
        //    cerr << "Read failed" << endl;
            continue;
        }
        
        string command(buffer);  // Convert buffer to string
        cout << "Client command: " << command << endl;

        // If client wants to exit, close the socket and break the loop
        if (command == "Exit") {
            close(client_socket);
            break;
        }

        stringstream ss(command);  // Parse the command
        string option;
        ss >> option;

        // Handle the "Newgraph" command to create a new graph with given vertices and edges
        if (option == "Newgraph") {
            int n, m;
            ss >> n >> m;  // Read the number of vertices (n) and edges (m)
            lock_guard<mutex> lock(graph_mutex);  // Lock the mutex to ensure thread-safe graph modification
            g = Graph(n);  // Create a new graph

            // Add 'm' edges to the graph
            for (int i = 0; i < m; ++i) {
                int u, v;
                ss >> u >> v;
                g.addEdge(u, v);  // Add edge from vertex 'u' to vertex 'v'
            }
        }
        // Handle the "Kosaraju" command to compute and print strongly connected components (SCCs)
        else if (option == "Kosaraju") {
            lock_guard<mutex> lock(graph_mutex);  // Lock the mutex for thread-safe SCC computation
            printSCCs(g);  // Print the SCCs using Kosaraju's algorithm
        }
        // Handle the "Newedge" command to add a new edge to the graph
        else if (option == "Newedge") {
            int u, v;
            ss >> u >> v;  // Read the edge to be added
            lock_guard<mutex> lock(graph_mutex);  // Lock the mutex for thread-safe edge addition
            g.addEdge(u, v);  // Add the edge to the graph
        }
        // Handle the "Removeedge" command to remove an edge from the graph
        else if (option == "Removeedge") {
            int u, v;
            ss >> u >> v;  // Read the edge to be removed
            lock_guard<mutex> lock(graph_mutex);  // Lock the mutex for thread-safe edge removal
            g.removeEdge(u, v);  // Remove the edge from the graph
        }
        // Handle invalid or unknown commands
        else {
            cout << "Invalid command: " << command << endl;
        }

        // Clear the buffer for the next command
        memset(buffer, 0, sizeof(buffer));
    }
}

int main() {
    cout << "Welcome to the server!" << endl;
    int server_fd, new_socket;
    struct sockaddr_in serverAddr, clientAddr;
    int addrlen = sizeof(clientAddr);

    // Create a socket for the server
    cout << "Creating server socket..." << endl;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        cerr << "Socket creation failed" << endl;
        return 1;
    }

    int opt = 1;
    // Set socket options to allow quick reuse of the address
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        cerr << "Setsockopt failed" << endl;
        return 1;
    }
    
    // Configure the server address (IPv4, port 9037)
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(9037);

    // Bind the server socket to the specified IP and port
    cout << "Binding server socket..." << endl;
    bind(server_fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    // Start listening for incoming client connections (max 3 queued connections)
    cout << "Listening for incoming connections..." << endl;
    listen(server_fd, 10);

    // Main server loop to accept and handle clients
    cout << "Starting Communication..." << endl;

    while (true) {

        // Accept a new client connection
        new_socket = accept(server_fd, (struct sockaddr *)&serverAddr, (socklen_t*)&addrlen);
        
        // Create a new thread to handle the client request
        pthread_t thread;
        int *client_socket = new int(new_socket);
        if (pthread_create(&thread, NULL, (void *(*)(void *))handleClient, client_socket) < 0) {        //safty check for thread creation
            cerr << "Thread creation failed" << endl;
            return 1;
        }

        // Detach the thread to allow it to run independently
        pthread_detach(thread);
    }

    // Close the server socket
    close(server_fd);
    return 0;
}
