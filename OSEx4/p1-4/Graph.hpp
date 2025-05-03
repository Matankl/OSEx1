#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <unordered_map>
#include <list>
#include <iostream>

class Graph {
private:
    std::unordered_map<int, std::list<int>> adjList; // Adjacency list for storing edges
    int vertices;                                   // Number of vertices in the graph

public:
    // Constructor to initialize the graph with a given number of vertices
    Graph(int v);

    // Function to add an edge between two vertices (undirected graph)
    void addEdge(int u, int v);

    // Function to display the graph
    void displayGraph() const;

    // Getter for the adjacency list
    const std::unordered_map<int, std::list<int>> &getAdjList() const;

    // Function to determine if the graph has an Eulerian circuit
    bool isEulerian() const;
};

#endif // GRAPH_HPP
