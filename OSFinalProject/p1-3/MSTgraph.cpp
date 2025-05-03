#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <limits>
#include <memory>     // for std::unique_ptr, std::make_unique

// ----------------------------------------------------
// 1. ENUM to specify which MST algorithm to use
// ----------------------------------------------------
enum class MSTType {
    KRUSKAL,
    PRIM
};

// ----------------------------------------------------
// 2. Forward declaration of Graph
// ----------------------------------------------------
class Graph;

// ----------------------------------------------------
// 3. MSTAlgorithm (Strategy interface)
//    - Abstract class with a pure virtual buildMST
// ----------------------------------------------------
class MSTAlgorithm {
public:
    virtual ~MSTAlgorithm() = default;

    // buildMST should set up the MST adjacency in the given Graph
    // and compute any needed MST metrics (like total weight).
    virtual void buildMST(Graph& graph) = 0;
};

// ----------------------------------------------------
// 4. The Graph Class
//    - Holds original edges and adjacency
//    - Holds MST adjacency once built
//    - Allows user to set the MST algorithm at runtime
//    - Provides BFS-based methods to compute diameter, etc.
// ----------------------------------------------------
class Graph {
private:
    int V;  // Number of vertices

    // Original graph data
    std::vector<std::tuple<int,int,double>> edges;      // (u, v, w)
    std::vector<std::vector<std::pair<int,double>>> adj; // adjacency for the original graph

    // MST data
    std::vector<std::vector<std::pair<int,double>>> mstAdj; // adjacency for the MST
    double mstWeight;    // total weight of MST
    bool mstBuilt;       // flag to check if MST is built

    // Current MST building strategy
    std::unique_ptr<MSTAlgorithm> mstStrategy;

public:
    // Constructor: Initialize with V vertices
    Graph(int vertices)
        : V(vertices), mstWeight(0.0), mstBuilt(false) 
    {
        // Resize adjacency for the original graph
        adj.resize(V);
    }

    // Getter for number of vertices
    int getNumVertices() const {
        return V;
    }

    // Add an edge to the *original* graph
    void addEdge(int u, int v, double w) {
        // Store in edge list (useful for Kruskal)
        edges.push_back({u, v, w});
        // Also add to adjacency (useful for Prim)
        adj[u].push_back({v, w});
        adj[v].push_back({u, w}); // undirected graph
    }

    // Expose the edges list (for Kruskal)
    const std::vector<std::tuple<int,int,double>>& getEdges() const {
        return edges;
    }

    // Expose the adjacency (for Prim)
    const std::vector<std::vector<std::pair<int,double>>>& getAdj() const {
        return adj;
    }

    // Methods to set/get MST adjacency
    void initMSTAdj() {
        mstAdj.clear();
        mstAdj.resize(V);
    }
    std::vector<std::vector<std::pair<int,double>>>& getMSTAdj() {
        return mstAdj;
    }

    // Mark MST as built + store weight
    void setMSTBuilt(double weight) {
        mstWeight = weight;
        mstBuilt  = true;
    }
    bool isMSTBuilt() const {
        return mstBuilt;
    }
    double getMSTWeight() const {
        return mstWeight;
    }

    // -------------- STRATEGY-RELATED --------------
    // setAlgorithm: choose Kruskal or Prim at runtime
    void setAlgorithm(std::unique_ptr<MSTAlgorithm> alg) {
        mstStrategy = std::move(alg);
    }

    // buildMST: calls the chosen strategy's buildMST
    void buildMST() {
        if (!mstStrategy) {
            std::cerr << "No MST algorithm selected!\n";
            return;
        }
        // If already built, skip if desired
        if (mstBuilt) return;

        mstStrategy->buildMST(*this);
    }

    // ------------------------------------------------
    // BFS-based methods for distances in MST
    // ------------------------------------------------

    // BFS in MST to get distances from a start vertex
    // (We call it BFS, but we handle weighted edges in a tree.
    //  There's only one path to each node, so a simple queue-based
    //  traversal can track the correct distances.)
    std::vector<double> bfsDistancesInMST(int start) {
        // Ensure MST is built
        if (!mstBuilt) {
            std::cerr << "MST not built yet, cannot compute distances.\n";
            return {};
        }

        std::vector<double> dist(V, std::numeric_limits<double>::infinity());
        dist[start] = 0.0;

        std::queue<int> q;
        q.push(start);

        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (auto &nbr : mstAdj[u]) {
                int nxt = nbr.first;
                double w = nbr.second;
                if (dist[u] + w < dist[nxt]) {
                    dist[nxt] = dist[u] + w;
                    q.push(nxt);
                }
            }
        }
        return dist;
    }

    // getLongestDistance: returns the diameter of the MST
    double getLongestDistance() {
        if (!mstBuilt) {
            std::cerr << "MST not built.\n";
            return 0.0;
        }
        // 1) BFS from node 0 (arbitrary) to find farthest node X
        auto distFrom0 = bfsDistancesInMST(0);
        int farthestNode = 0;
        for (int i = 0; i < V; i++) {
            if (distFrom0[i] > distFrom0[farthestNode]) {
                farthestNode = i;
            }
        }
        // 2) BFS from X to find farthest node Y
        auto distFromX = bfsDistancesInMST(farthestNode);
        double diameter = 0.0;
        for (double d : distFromX) {
            diameter = std::max(diameter, d);
        }
        return diameter;
    }

    // getAverageDistance: average pairwise distance in the MST
    // including (i, i) pairs, which are distance 0.
    double getAverageDistance() {
        if (!mstBuilt) {
            std::cerr << "MST not built.\n";
            return 0.0;
        }

        long long pairCount = (long long)V * (long long)V;
        double sumDist = 0.0;

        for (int i = 0; i < V; i++) {
            auto dist = bfsDistancesInMST(i);
            for (double d : dist) {
                sumDist += d;
            }
        }
        return sumDist / static_cast<double>(pairCount);
    }
};

// ----------------------------------------------------
// 5. KruskalMST class (implements MSTAlgorithm)
// ----------------------------------------------------
class KruskalMST : public MSTAlgorithm {
private:
    // Disjoint Set structure for cycle checks in Kruskal
    class DisjointSet {
    private:
        std::vector<int> parent, rankSet;
    public:
        DisjointSet(int n) {
            parent.resize(n);
            rankSet.resize(n, 0);
            for(int i = 0; i < n; i++) {
                parent[i] = i;
            }
        }

        int find(int u) {
            if(parent[u] != u) {
                parent[u] = find(parent[u]);
            }
            return parent[u];
        }

        void unite(int u, int v) {
            int rootU = find(u);
            int rootV = find(v);
            if(rootU != rootV) {
                if(rankSet[rootU] > rankSet[rootV]) {
                    parent[rootV] = rootU;
                } else if(rankSet[rootU] < rankSet[rootV]) {
                    parent[rootU] = rootV;
                } else {
                    parent[rootV] = rootU;
                    rankSet[rootU]++;
                }
            }
        }
    };

public:
    // buildMST using Kruskal's algorithm
    void buildMST(Graph& graph) override {
        int V = graph.getNumVertices();
        auto &edges = graph.getEdges();  // all edges in the original graph

        // Sort edges by ascending weight
        std::vector<std::tuple<int,int,double>> sortedEdges(edges.begin(), edges.end());
        std::sort(sortedEdges.begin(), sortedEdges.end(),
                  [](auto &e1, auto &e2){
                      return std::get<2>(e1) < std::get<2>(e2);
                  });

        DisjointSet ds(V);
        graph.initMSTAdj(); // clear & resize MST adjacency
        double totalWeight = 0.0;
        int edgesUsed = 0;

        for (auto &e : sortedEdges) {
            int u = std::get<0>(e);
            int v = std::get<1>(e);
            double w = std::get<2>(e);

            // If u,v are in different sets, add this edge
            if (ds.find(u) != ds.find(v)) {
                ds.unite(u, v);
                // Add to MST adjacency
                graph.getMSTAdj()[u].push_back({v, w});
                graph.getMSTAdj()[v].push_back({u, w});

                totalWeight += w;
                edgesUsed++;
                // If we have V-1 edges, MST is complete
                if (edgesUsed == V-1) break;
            }
        }

        // Mark MST built in the Graph
        graph.setMSTBuilt(totalWeight);
    }
};

// ----------------------------------------------------
// 6. PrimMST class (implements MSTAlgorithm)
// ----------------------------------------------------
class PrimMST : public MSTAlgorithm {
public:
    // buildMST using Prim's algorithm (with a min-heap / priority queue)
    void buildMST(Graph& graph) override {
        int V = graph.getNumVertices();
        auto &originalAdj = graph.getAdj();

        graph.initMSTAdj(); // prepare MST adjacency
        std::vector<std::vector<std::pair<int,double>>>& mstAdj = graph.getMSTAdj();

        // Array to track visited nodes
        std::vector<bool> inMST(V, false);

        // Min-heap storing (weight, node, parentInMST)
        //   parentInMST is the node from which we discovered this vertex
        //   so we can record the edge later.
        using EdgeInfo = std::tuple<double,int,int>; 
        std::priority_queue<EdgeInfo, std::vector<EdgeInfo>, std::greater<EdgeInfo>> pq;

        double totalWeight = 0.0;
        int edgesUsed = 0;

        // Start from node 0 (arbitrary). Mark it and push its edges to the queue
        inMST[0] = true;
        for (auto &nbr : originalAdj[0]) {
            // (weight, neighbor, 0)
            pq.push({nbr.second, nbr.first, 0});
        }

        // Repeat until we have V-1 edges or the queue is empty
        while (!pq.empty() && edgesUsed < V-1) {
            auto [w, node, parent] = pq.top();
            pq.pop();

            // If this node is already in MST, skip
            if (inMST[node]) continue;

            // Otherwise, add it to MST
            inMST[node] = true;
            edgesUsed++;
            totalWeight += w;

            // Record the edge in the MST adjacency
            // (parent - node) with weight w
            mstAdj[parent].push_back({node, w});
            mstAdj[node].push_back({parent, w});

            // Push all edges from 'node' to unvisited neighbors
            for (auto &nbr : originalAdj[node]) {
                int nxt = nbr.first;
                double nxtW = nbr.second;
                if (!inMST[nxt]) {
                    pq.push({nxtW, nxt, node});
                }
            }
        }

        graph.setMSTBuilt(totalWeight);
    }
};

// ----------------------------------------------------
// 7. MSTFactory: returns KruskalMST or PrimMST
// ----------------------------------------------------
class MSTFactory {
public:
    static std::unique_ptr<MSTAlgorithm> createMSTAlgorithm(MSTType type) {
        switch(type) {
        case MSTType::KRUSKAL:
            return std::make_unique<KruskalMST>();
        case MSTType::PRIM:
            return std::make_unique<PrimMST>();
        default:
            // Fallback - default to Kruskal
            return std::make_unique<KruskalMST>();
        }
    }
};

// ----------------------------------------------------
// 8. Example Main
// ----------------------------------------------------
int main() {
    // Create a graph with 5 vertices (0..4)
    Graph g(5);

    // Add edges: (u, v, weight)
    g.addEdge(0, 1, 2.0);
    g.addEdge(0, 2, 3.0);
    g.addEdge(1, 2, 1.0);
    g.addEdge(1, 3, 4.0);
    g.addEdge(2, 4, 5.0);
    g.addEdge(3, 4, 1.0);

    // --- Choose MST algorithm via the factory ---
    //pick Kruskal first
    std::unique_ptr<MSTAlgorithm> mstAlg = MSTFactory::createMSTAlgorithm(MSTType::KRUSKAL);
    // Set it in the graph
    g.setAlgorithm(std::move(mstAlg));

    // Build MST
    g.buildMST();

    // Display MST total weight
    std::cout << "Using Kruskal:\n";
    std::cout << "  - MST Total Weight = " << g.getMSTWeight() << "\n";
    std::cout << "  - MST Diameter     = " << g.getLongestDistance() << "\n";
    std::cout << "  - MST Avg Distance = " << g.getAverageDistance() << "\n\n";

    // ---- If we want to use Prim, we can do a new Graph or reset this one ----
    // We'll create another graph with the same edges for demonstration
    Graph g2(5);
    g2.addEdge(0, 1, 2.0);
    g2.addEdge(0, 2, 3.0);
    g2.addEdge(1, 2, 1.0);
    g2.addEdge(1, 3, 4.0);
    g2.addEdge(2, 4, 5.0);
    g2.addEdge(3, 4, 1.0);

    // Now pick Prim
    auto mstAlgPrim = MSTFactory::createMSTAlgorithm(MSTType::PRIM);
    g2.setAlgorithm(std::move(mstAlgPrim));
    g2.buildMST();

    std::cout << "Using Prim:\n";
    std::cout << "  - MST Total Weight = " << g2.getMSTWeight() << "\n";
    std::cout << "  - MST Diameter     = " << g2.getLongestDistance() << "\n";
    std::cout << "  - MST Avg Distance = " << g2.getAverageDistance() << "\n";

    return 0;
}
