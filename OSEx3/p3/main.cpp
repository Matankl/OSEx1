#include <iostream>
#include <deque>
#include <vector>
#include <algorithm>
#include <sstream>

using namespace std;

class Graph {
    int n;  // Number of vertices
    vector<deque<int>> adj;  // Adjacency list using deque

public:
    Graph(int vertices) : n(vertices) {
        adj.resize(n + 1);  // Resize adjacency list for 1-based indexing
    }

    // Function to add an edge from vertex 'u' to vertex 'v'
    void addEdge(int u, int v) {
        adj[u].push_back(v);  // Add edge u -> v in the original graph
    }

    // Function to remove an edge from vertex 'u' to vertex 'v'
    void removeEdge(int u, int v) {
        // Find the edge v in the adjacency list of u
        auto it = find(adj[u].begin(), adj[u].end(), v);
        if (it != adj[u].end()) {
            adj[u].erase(it);  // Erase the edge v from the adjacency list of u
        }
    }

    // Get the number of vertices
    int getNumVertices() const {
        return n;
    }

    // Get the adjacency list of the graph
    const deque<int>& getAdjList(int v) const {
        return adj[v];
    }

    // Function to create and return the transposed graph (reverse edges)
    Graph transposeGraph() const {
        Graph transposed(n);  // Create a new graph with the same number of vertices
        
        // Reverse all edges from the original graph
        for (int u = 1; u <= n; ++u) {
            for (int v : adj[u]) {
                transposed.addEdge(v, u);  // Reverse edge u -> v becomes v -> u
            }
        }

        return transposed;
    }
};

// Helper function to perform DFS and fill the deque with vertices in order of completion time
void fillOrder(const Graph &g, int v, vector<bool> &visited, deque<int> &Stack) {
    visited[v] = true;  // Mark the current vertex as visited
    
    // Recur for all adjacent vertices in the original graph
    for (int i : g.getAdjList(v)) {
        if (!visited[i]) {
            fillOrder(g, i, visited, Stack);
        }
    }
    
    // Push the current vertex to the deque after visiting all its neighbors
    Stack.push_back(v);
}

// A DFS function to explore all vertices in the reversed graph
void dfs(const Graph &g, int v, vector<bool> &visited, deque<int> &component) {
    visited[v] = true;   // Mark the current vertex as visited
    component.push_back(v);  // Add the current vertex to the current component

    // Recur for all adjacent vertices in the reversed graph
    for (int i : g.getAdjList(v)) {
        if (!visited[i]) {
            dfs(g, i, visited, component);
        }
    }
}

// Function to print the strongly connected components (SCCs) using Kosaraju's algorithm
void printSCCs(const Graph &g) {
    int n = g.getNumVertices();
    deque<int> Stack;
    vector<bool> visited(n + 1, false);  // Initialize visited array for the first DFS

    // Step 1: Perform DFS on the original graph to fill the deque
    for (int i = 1; i <= n; ++i) {
        if (!visited[i]) {
            fillOrder(g, i, visited, Stack);
        }
    }

    // Step 2: Get the transposed graph
    Graph transposed = g.transposeGraph();

    // Step 3: Reset the visited array for the second DFS
    fill(visited.begin(), visited.end(), false);

    // Step 4: Process vertices in order of decreasing finishing time (from deque)
    while (!Stack.empty()) {
        int v = Stack.back();
        Stack.pop_back();

        // If this vertex hasn't been visited, it's part of a new SCC
        if (!visited[v]) {
            deque<int> component;  // Stores the current SCC
            dfs(transposed, v, visited, component);  // Perform DFS on reversed graph for this SCC

            // Print the current strongly connected component
            for (int vertex : component) {
                cout << vertex << " ";
            }
            cout << endl;  // Newline after each SCC
        }
    }
}

int main() {
    // options for use
    int n, m;
    Graph g = Graph(0);  // Create a graph with '0' vertices
    std::string option;
    std::string indexs;
    char comma;
    while (1) {
        std::cin >> option;
        std::cout << "Option: " << option << std::endl;                     // debug prit delete later
        if (option != "Kosaraju" && option != "Exit")
        {
            std::cin >> indexs;
            std::stringstream ss(indexs);
            ss >> n >> comma >> m;  // Use stringstream to extract n, comma, and m

            if (ss && comma == ',') {
                std::cout << "Parsed n/i: " << n << ", m/j: " << m << std::endl;
            } else {
                std::cout << "Invalid format. Please enter in n,m format." << std::endl;           // i can see the future problems 
            }
        }
        
        if (option == "Exit") {
            break;
        }
        // Parse the input string
        
        // Back to the options 
        if (option == "Newgraph") {
            Graph newGraph(n); // A temporary new graph
            g = std::move(newGraph); // Move the new graph into the existing one

            // Input: Read the 'm' edges
            for (int i = 0; i < m; ++i) {
                int u, v;
                cin >> u >> v;  // Read edge from vertex u to vertex v
                g.addEdge(u, v);  // Add the edge to the graph
            }
        }
        else if (option == "Kosaraju") {
            if (g.getNumVertices() > 0) {
                // Output: Print the strongly connected components (SCCs)
                printSCCs(g);
            }
            else {
                std::cout << "No graph found. Please create a new graph using command 'Newgraph n,m'." << std::endl;               // i can see the future problems
            }
        }
        else if (option == "Newedge"){
            if (g.getNumVertices() > 0) {
                g.addEdge(n, m);
            }
            else {
                std::cout << "No graph found. Please create a new graph using command 'Newgraph n,m'." << std::endl;               // i can see the future problems
            }
        }
        else if (option == "Removeedge"){
            if (g.getNumVertices() > 0) {
                g.removeEdge(n, m);
            }
            else {
                std::cout << "No graph found. Please create a new graph using command 'Newgraph n,m'." << std::endl;               // i can see the future problems
            }
        }
        else {
            std::cout << "Invalid option. Please enter a valid option." << std::endl;                   // i can see the future problems
        }
    }


    return 0;
}
