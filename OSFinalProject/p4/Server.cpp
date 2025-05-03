#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include <queue>
#include <algorithm>
#include <memory>
#include <cstring>      // memset
#include <unistd.h>     // close, read, write
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// --------------- MSTType enum ---------------
enum class MSTType {
    KRUSKAL,
    PRIM
};

class Graph;           // forward declaration
class MSTAlgorithm;    // forward declaration

// ---------------- MSTAlgorithm interface ----------------
class MSTAlgorithm {
public:
    virtual ~MSTAlgorithm() = default;
    virtual void buildMST(Graph& graph) = 0;
};

// ---------------- The Graph class ----------------
class Graph {
private:
    int V; // number of vertices

    // Original graph data
    std::vector<std::tuple<int,int,double>> edges;  // (u, v, w)
    std::vector<std::vector<std::pair<int,double>>> adj; // adjacency list for original graph

    // MST adjacency
    std::vector<std::vector<std::pair<int,double>>> mstAdj; 
    double mstWeight;
    bool   mstBuilt;

    // Chosen MST strategy
    std::unique_ptr<MSTAlgorithm> strategy;

public:
    Graph(int vertices=0) : V(vertices), mstWeight(0.0), mstBuilt(false) {
        adj.resize(V);
    }

    void createGraph(int vertices) {
        V = vertices;
        edges.clear();
        adj.clear();
        adj.resize(V);
        mstAdj.clear();
        mstWeight = 0.0;
        mstBuilt = false;
        strategy.reset();
    }

    int getNumVertices() const {
        return V;
    }

    void addEdge(int u, int v, double w) {
        if(u<0 || v<0 || u>=V || v>=V) return; // ignore invalid
        edges.push_back({u,v,w});
        adj[u].push_back({v,w});
        adj[v].push_back({u,w});
        mstBuilt = false;
        mstAdj.clear();
    }

    const std::vector<std::tuple<int,int,double>>& getEdges() const {
        return edges;
    }

    const std::vector<std::vector<std::pair<int,double>>>& getAdj() const {
        return adj;
    }

    // Strategy
    void setAlgorithm(std::unique_ptr<MSTAlgorithm> alg) {
        strategy = std::move(alg);
        mstBuilt = false;
        mstAdj.clear();
    }

    // Build MST
    void buildMST() {
        if(!strategy) {
            std::cerr << "[Server] No MST algorithm set.\n";
            return;
        }
        if(mstBuilt) return; // already built
        strategy->buildMST(*this);
        mstBuilt = true;
    }

    bool isMSTBuilt() const {
        return mstBuilt;
    }

    // MST adjacency access
    void initMSTAdj() {
        mstAdj.clear();
        mstAdj.resize(V);
    }
    std::vector<std::vector<std::pair<int,double>>>& getMSTAdj() {
        return mstAdj;
    }

    // Mark MST built
    void setMSTBuilt(double totalWeight) {
        mstWeight = totalWeight;
        mstBuilt = true;
    }
    double getMSTWeight() const {
        if(!mstBuilt) return -1.0;
        return mstWeight;
    }

    // BFS in MST
    std::vector<double> bfsDistances(int start) {
        std::vector<double> dist(V, 1e15);
        dist[start] = 0.0;
        std::queue<int>q;
        q.push(start);

        while(!q.empty()) {
            int u = q.front(); q.pop();
            for(auto &nbr : mstAdj[u]) {
                int nxt = nbr.first;
                double w = nbr.second;
                if(dist[u] + w < dist[nxt]) {
                    dist[nxt] = dist[u] + w;
                    q.push(nxt);
                }
            }
        }
        return dist;
    }

    // MST diameter
    double getLongestDistance() {
        if(!mstBuilt || V==0) return 0.0;
        // BFS from 0
        auto d0 = bfsDistances(0);
        int farthest=0;
        for(int i=0; i<V; i++){
            if(d0[i]>d0[farthest]) farthest=i;
        }
        // BFS from farthest
        auto d1 = bfsDistances(farthest);
        double diameter=0.0;
        for(double x: d1) {
            if(x>diameter) diameter=x;
        }
        return diameter;
    }

    // MST average distance
    double getAverageDistance() {
        if(!mstBuilt || V<=0) return 0.0;
        long long pairCount = (long long)V*V;
        double sum=0.0;
        for(int i=0; i<V; i++){
            auto dist = bfsDistances(i);
            for(double d: dist){
                sum+=d;
            }
        }
        return sum / (double)pairCount;
    }
};

// --------------- KruskalMST ---------------
class KruskalMST : public MSTAlgorithm {
private:
    class DisjointSet {
    private:
        std::vector<int> parent, rankv;
    public:
        DisjointSet(int n) {
            parent.resize(n); rankv.resize(n,0);
            for(int i=0;i<n;i++) parent[i]=i;
        }
        int find(int u) {
            if(parent[u]!=u) parent[u]=find(parent[u]);
            return parent[u];
        }
        void unify(int u, int v) {
            int ru=find(u), rv=find(v);
            if(ru!=rv) {
                if(rankv[ru]>rankv[rv]) parent[rv]=ru;
                else if(rankv[ru]<rankv[rv]) parent[ru]=rv;
                else {
                    parent[rv]=ru;
                    rankv[ru]++;
                }
            }
        }
    };
public:
    void buildMST(Graph &graph) override {
        int V=graph.getNumVertices();
        auto &allEdges=graph.getEdges();

        // Sort edges
        std::vector<std::tuple<int,int,double>> edges(allEdges.begin(), allEdges.end());
        std::sort(edges.begin(), edges.end(),
                  [](auto &a, auto &b){
                      return std::get<2>(a) < std::get<2>(b);
                  });

        graph.initMSTAdj();
        DisjointSet ds(V);
        double totalWeight=0.0;
        int used=0;

        for(auto &e: edges) {
            int u=std::get<0>(e), v=std::get<1>(e);
            double w=std::get<2>(e);
            if(ds.find(u)!=ds.find(v)) {
                ds.unify(u,v);
                graph.getMSTAdj()[u].push_back({v,w});
                graph.getMSTAdj()[v].push_back({u,w});
                totalWeight+=w;
                used++;
                if(used==V-1) break;
            }
        }
        graph.setMSTBuilt(totalWeight);
    }
};

// --------------- PrimMST ---------------
class PrimMST : public MSTAlgorithm {
public:
    void buildMST(Graph &graph) override {
        int V=graph.getNumVertices();
        auto &origAdj=graph.getAdj();
        graph.initMSTAdj();
        auto &mstAdj = graph.getMSTAdj();

        if(V==0) {
            graph.setMSTBuilt(0.0);
            return;
        }

        // We'll store (weight, node, parent) in the PQ
        using EdgeTuple=std::tuple<double,int,int>;
        std::priority_queue<EdgeTuple, std::vector<EdgeTuple>, std::greater<EdgeTuple>> pq;
        std::vector<bool> inMST(V,false);
        double totalWeight=0.0;
        int edgesUsed=0;

        // Start from node 0
        inMST[0]=true;
        for(auto &nbr: origAdj[0]) {
            // Instead of pq.push({nbr.second, nbr.first, 0}), do:
            pq.emplace(nbr.second, nbr.first, 0);
        }

        while(!pq.empty() && edgesUsed<(V-1)) {
            auto [w,node,parent] = pq.top();
            pq.pop();
            if(inMST[node]) continue;
            inMST[node]=true;
            edgesUsed++;
            totalWeight += w;
            mstAdj[parent].push_back({node,w});
            mstAdj[node].push_back({parent,w});

            // expand from 'node'
            for(auto &nbr: origAdj[node]) {
                if(!inMST[nbr.first]) {
                    pq.emplace(nbr.second, nbr.first, node);
                }
            }
        }

        graph.setMSTBuilt(totalWeight);
    }
};

// --------------- Factory Helper ---------------
std::unique_ptr<MSTAlgorithm> createMSTAlgorithm(const std::string &algName) {
    if(algName=="KRUSKAL") {
        return std::make_unique<KruskalMST>();
    } else if(algName=="PRIM") {
        return std::make_unique<PrimMST>();
    }
    return nullptr;
}

// ========================================================================
//  SERVER-SPECIFIC CODE
// ========================================================================

// We'll maintain a single global Graph
static Graph globalGraph;

static std::string trim(const std::string &s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if(start==std::string::npos) return "";
    auto end   = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end-start+1);
}

// handleClient
void handleClient(int clientSockFD) {
    constexpr size_t BUF_SIZE=1024;
    char buffer[BUF_SIZE];

    while(true) {
        memset(buffer,0,BUF_SIZE);
        int bytesRead=read(clientSockFD, buffer, BUF_SIZE-1);
        if(bytesRead<=0) {
            std::cerr << "[Server] Client disconnected.\n";
            break;
        }
        std::string line=trim(std::string(buffer));
        if(line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;
        for(auto &c: cmd) c=toupper(c);

        std::string response;

        if(cmd=="QUIT" || cmd=="EXIT") {
            response="Goodbye!\n";
            write(clientSockFD, response.c_str(), response.size());
            break;
        } else if(cmd=="CREATE_GRAPH") {
            int v;
            ss >> v;
            globalGraph.createGraph(v);
            response="Graph created with "+std::to_string(v)+" vertices.\n";
        } else if(cmd=="ADD_EDGE") {
            int u,v;
            double w;
            ss >> u >> v >> w;
            globalGraph.addEdge(u,v,w);
            response="Edge added: ("+std::to_string(u)+","+std::to_string(v)+") weight="+std::to_string(w)+"\n";
        } else if(cmd=="SET_ALG") {
            std::string algName;
            ss >> algName;
            for(auto &ch: algName) ch=toupper(ch);
            auto alg = createMSTAlgorithm(algName);
            if(!alg) {
                response="Unknown algorithm: "+algName+"\n";
            } else {
                globalGraph.setAlgorithm(std::move(alg));
                response="Algorithm set to "+algName+"\n";
            }
        } else if(cmd=="BUILD_MST") {
            globalGraph.buildMST();
            if(!globalGraph.isMSTBuilt()) {
                response="Failed to build MST.\n";
            } else {
                response="MST built.\n";
            }
        } else if(cmd=="GET_WEIGHT") {
            double w=globalGraph.getMSTWeight();
            if(w<0.0) {
                response="MST not built yet.\n";
            } else {
                response="MST Weight="+std::to_string(w)+"\n";
            }
        } else if(cmd=="GET_DIAMETER") {
            if(!globalGraph.isMSTBuilt()) {
                response="MST not built yet.\n";
            } else {
                double d=globalGraph.getLongestDistance();
                response="MST Diameter="+std::to_string(d)+"\n";
            }
        } else if(cmd=="GET_AVG_DIST") {
            if(!globalGraph.isMSTBuilt()) {
                response="MST not built yet.\n";
            } else {
                double avg=globalGraph.getAverageDistance();
                response="MST AvgDist="+std::to_string(avg)+"\n";
            }
        } else {
            response="Unknown command: "+cmd+"\n";
        }

        write(clientSockFD, response.c_str(), response.size());
    }

    close(clientSockFD);
}

int main() {
    int port=12345;
    int serverFD=socket(AF_INET, SOCK_STREAM, 0);
    if(serverFD<0) {
        std::cerr<<"[Server] socket() failed\n";
        return 1;
    }
    int opt=1;
    setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family=AF_INET;
    servAddr.sin_addr.s_addr=INADDR_ANY;
    servAddr.sin_port=htons(port);

    if(bind(serverFD, (sockaddr*)&servAddr, sizeof(servAddr))<0){
        std::cerr<<"[Server] bind() failed\n";
        close(serverFD);
        return 1;
    }
    if(listen(serverFD,1)<0){
        std::cerr<<"[Server] listen() failed\n";
        close(serverFD);
        return 1;
    }
    std::cout<<"[Server] Listening on port "<<port<<"...\n";

    sockaddr_in clientAddr;
    socklen_t clientLen=sizeof(clientAddr);
    int clientFD=accept(serverFD,(sockaddr*)&clientAddr,&clientLen);
    if(clientFD<0) {
        std::cerr<<"[Server] accept() failed\n";
        close(serverFD);
        return 1;
    }
    std::cout<<"[Server] Client connected.\n";

    handleClient(clientFD);

    close(serverFD);
    return 0;
}
