#include "Graph.hpp"

// Constructor to initialize the graph with a given number of vertices
Graph::Graph(int v) : vertices(v) {}

// Function to add an edge between two vertices (undirected graph)
void Graph::addEdge(int u, int v) {
    adjList[u].push_back(v); // Add v to u's list
    adjList[v].push_back(u); // Add u to v's list
}

// Function to display the graph
void Graph::displayGraph() const {
    for (auto &pair : adjList) {
        std::cout << pair.first << ": ";
        for (int neighbor : pair.second) {
            std::cout << neighbor << " ";
        }
        std::cout << std::endl;
    }
}

// Getter for the adjacency list
const std::unordered_map<int, std::list<int>> &Graph::getAdjList() const {
    return adjList;
}

// Function to determine if a graph has an Eulerian circuit
bool Graph::isEulerian() const {
    // Get adjacency list of the graph
    const auto &adjList = getAdjList();

    // Map to track visited vertices
    std::unordered_map<int, bool> visited;
    for (const auto &pair : adjList) {
        visited[pair.first] = false; // Initialize all vertices as unvisited
    }

    // Lambda function for Depth First Search (DFS)
    auto dfs = [&](int node, auto &&dfsRef) -> void {
        visited[node] = true; // Mark the current node as visited
        for (int neighbor : adjList.at(node)) {
            if (!visited[neighbor]) {
                dfsRef(neighbor, dfsRef); // Recursively visit neighbors
            }
        }
    };

    // Find the first vertex with edges to start the DFS
    int startVertex = -1;
    for (const auto &pair : adjList) {
        if (!pair.second.empty()) { // Vertex has at least one edge
            startVertex = pair.first;
            break;
        }
    }

    // If no edges exist, the graph is trivially Eulerian
    if (startVertex == -1) return true;

    // Perform DFS to check connectivity
    dfs(startVertex, dfs);

    // Ensure all vertices with edges are visited
    for (const auto &pair : adjList) {
        if (!pair.second.empty() && !visited[pair.first]) {
            return false; // Not connected
        }
    }

    // Check if all vertices have an even degree
    for (const auto &pair : adjList) {
        if (pair.second.size() % 2 != 0) {
            return false; // Vertex has an odd degree
        }
    }

    return true; // All conditions satisfied, Eulerian circuit exists
}


/*
An Eulerian circle (also called an Eulerian circuit) is a cycle within a graph that:

    Starts and ends on the same vertex, and
    Uses every edge of the graph exactly once.

Equivalently, it’s a path that traverses each edge exactly one time before returning to the starting point.
In an undirected, connected graph, an Eulerian circle exists if and only if every vertex has an even degree.

*/