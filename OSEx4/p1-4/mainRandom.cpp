#include "Graph.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h> // For getopt

void printUsage(const char *programName) {
    std::cout << "Usage: " << programName << " -v <vertices> -e <edges> -s <seed>\n";
    std::cout << "  -v: Number of vertices\n";
    std::cout << "  -e: Number of edges\n";
    std::cout << "  -s: Random seed (optional)\n";
}

int main(int argc, char *argv[]) {
    int vertices = 0, edges = 0, seed = time(nullptr); // Default seed is current time
    int opt;

    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "v:e:s:")) != -1) {
        switch (opt) {
            case 'v':
                vertices = std::atoi(optarg);
                break;
            case 'e':
                edges = std::atoi(optarg);
                break;
            case 's':
                seed = std::atoi(optarg);
                break;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }
    // in case of missing arguments or invalid values
    if (vertices <= 0 || edges < 0) {
        printUsage(argv[0]);
        return 1;
    }

    // Seed the random number generator
    std::srand(seed);

    // Create a graph with the specified number of vertices
    Graph graph(vertices);

    // Generate random edges
    for (int i = 0; i < edges; ++i) {
        int u = std::rand() % vertices;
        int v = std::rand() % vertices;
        while (v == u) { // Avoid self-loops
            v = std::rand() % vertices;
        }
        graph.addEdge(u, v);
    }

    // Display the generated graph
    std::cout << "Generated Graph:\n";
    graph.displayGraph();

    // Check if the graph has an Eulerian circuit
    if (graph.isEulerian()) {
        std::cout << "The graph has an Eulerian circuit.\n";
    } else {
        std::cout << "The graph does NOT have an Eulerian circuit.\n";
    }

    return 0;
}
