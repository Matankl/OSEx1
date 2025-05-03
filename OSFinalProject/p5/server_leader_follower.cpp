/***************************************************************
 * server_leader_follower.cpp
 * Demonstrates a simplified Leader-Follower thread pool pattern.
 *
 * We have multiple threads. One of them is the "leader" that calls
 * accept(). When it gets a connection, it hands off the connection
 * to be processed (potentially by the same or another thread) and
 * another thread becomes leader.
 *
 * This example is simplified. A real leader-follower might revolve
 * around an event loop or use non-blocking I/O. The essence is to
 * show the concurrency pattern with a pool of threads rotating roles.
 ***************************************************************/

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unistd.h>   // close(), read(), write()
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <sstream>
#include <memory>
#include <functional>
#include <atomic>

// MST code as before (Graph, MSTAlgorithm, KruskalMST, PrimMST).
// We'll reuse a global Graph for simplicity, or you might store
// per-connection state, etc.

class MSTAlgorithm {
public:
    virtual ~MSTAlgorithm()=default;
    virtual void buildMST(class Graph& g)=0;
};

// ========== Graph Class (simplified) ==========
class Graph {
private:
    int V;
    std::vector<std::tuple<int,int,double>> edges;      // (u, v, w)
    std::vector<std::vector<std::pair<int,double>>> adj; // adjacency list for the original graph

    std::vector<std::vector<std::pair<int,double>>> mstAdj;
    double mstWeight;
    bool   mstBuilt;

    std::unique_ptr<MSTAlgorithm> mstStrategy;

public:
    Graph(int vertices = 0)
        : V(vertices), mstWeight(0.0), mstBuilt(false)
    {
        adj.resize(V);
    }

    void createGraph(int vertices) {
        V = vertices;
        edges.clear();
        adj.clear();
        adj.resize(V);
        mstAdj.clear();
        mstWeight = 0.0;
        mstBuilt  = false;
        mstStrategy.reset();
    }

    int getNumVertices() const { return V; }

    void addEdge(int u, int v, double w) {
        if(u<0 || v<0 || u>=V || v>=V) return;
        edges.push_back({u,v,w});
        adj[u].push_back({v,w});
        adj[v].push_back({u,w});
        // Invalidate MST
        mstBuilt = false;
        mstAdj.clear();
    }

    void setAlgorithm(std::unique_ptr<MSTAlgorithm> alg) {
        mstStrategy = std::move(alg);
        mstBuilt = false;
        mstAdj.clear();
    }

    void buildMST() {
        if(!mstStrategy) {
            std::cerr << "[Server] No MST algorithm set.\n";
            return;
        }
        if(mstBuilt) return; // already built
        mstStrategy->buildMST(*this);
        mstBuilt = true;
    }

    bool isMSTBuilt() const { return mstBuilt; }
    double getMSTWeight() const { return mstBuilt ? mstWeight : -1.0; }
    void setMSTBuilt(double w) { mstWeight = w; mstBuilt = true; }
    std::vector<std::vector<std::pair<int,double>>>& getMSTAdj() { return mstAdj; }
    void initMSTAdj() { mstAdj.clear(); mstAdj.resize(V); }

    const std::vector<std::tuple<int,int,double>>& getEdges() const { return edges; }
    const std::vector<std::vector<std::pair<int,double>>>& getAdj() const { return adj; }

    // BFS in MST
    std::vector<double> bfsDistances(int start) {
        std::vector<double> dist(V, 1e15);
        dist[start] = 0.0;
        std::queue<int>q;
        q.push(start);

        while(!q.empty()) {
            int u = q.front(); q.pop();
            for(auto &pr : mstAdj[u]) {
                int nxt = pr.first;
                double w = pr.second;
                if(dist[u] + w < dist[nxt]) {
                    dist[nxt] = dist[u] + w;
                    q.push(nxt);
                }
            }
        }
        return dist;
    }

    double getLongestDistance() {
        if(!mstBuilt || V==0) return 0.0;
        auto d0 = bfsDistances(0);
        int farthest = 0;
        for(int i=0; i<V; i++) {
            if(d0[i] > d0[farthest]) farthest = i;
        }
        auto d1 = bfsDistances(farthest);
        double diam = 0.0;
        for(auto dd : d1) { if(dd>diam) diam=dd; }
        return diam;
    }

    double getAverageDistance() {
        if(!mstBuilt || V==0) return 0.0;
        long long pairCount = (long long)V*(long long)V;
        double sum = 0.0;
        for(int i=0; i<V; i++){
            auto dist = bfsDistances(i);
            for(auto d : dist) sum += d;
        }
        return sum / (double)pairCount;
    }
};

// ====== Kruskal Implementation ======
class KruskalMST : public MSTAlgorithm {
private:
    class DisjointSet {
    private:
        std::vector<int> parent, rankSet;
    public:
        DisjointSet(int n){
            parent.resize(n); rankSet.resize(n,0);
            for(int i=0;i<n;i++) parent[i]=i;
        }
        int find(int u){
            if(parent[u]!=u) parent[u]=find(parent[u]);
            return parent[u];
        }
        void unify(int u,int v){
            int ru=find(u),rv=find(v);
            if(ru!=rv){
                if(rankSet[ru]>rankSet[rv]) parent[rv]=ru;
                else if(rankSet[ru]<rankSet[rv]) parent[ru]=rv;
                else {parent[rv]=ru; rankSet[ru]++;}
            }
        }
    };
public:
    void buildMST(Graph &g) override {
        int V = g.getNumVertices();
        auto &allEdges = g.getEdges();
        std::vector<std::tuple<int,int,double>> edges(allEdges.begin(), allEdges.end());
        std::sort(edges.begin(), edges.end(), [](auto &e1, auto &e2){
            return std::get<2>(e1)<std::get<2>(e2);
        });
        g.initMSTAdj();
        double totalW = 0.0;
        DisjointSet ds(V);
        int used=0;
        for(auto &e : edges){
            int u=std::get<0>(e),v=std::get<1>(e);
            double w=std::get<2>(e);
            if(ds.find(u)!=ds.find(v)){
                ds.unify(u,v);
                g.getMSTAdj()[u].push_back({v,w});
                g.getMSTAdj()[v].push_back({u,w});
                totalW+=w;
                used++;
                if(used==V-1) break;
            }
        }
        g.setMSTBuilt(totalW);
    }
};

// ====== Prim Implementation ======
class PrimMST : public MSTAlgorithm {
public:
    void buildMST(Graph &g) override {
        int V=g.getNumVertices();
        if(V==0){
            g.setMSTBuilt(0.0);
            return;
        }
        g.initMSTAdj();
        auto &mstAdj = g.getMSTAdj();
        auto &origAdj= g.getAdj();

        std::vector<bool> inMST(V,false);
        typedef std::tuple<double,int,int> EdgeInfo;
        std::priority_queue<EdgeInfo,std::vector<EdgeInfo>,std::greater<EdgeInfo>> pq;

        inMST[0] = true;
        for(auto &nbr : origAdj[0]){
            pq.push({nbr.second,nbr.first,0});
        }
        double totalW=0.0;
        int edgesUsed=0;

        while(!pq.empty() && edgesUsed<(V-1)){
            auto [w,node,par] = pq.top(); pq.pop();
            if(inMST[node]) continue;
            inMST[node]=true;
            edgesUsed++;
            totalW+=w;
            mstAdj[par].push_back({node,w});
            mstAdj[node].push_back({par,w});

            for(auto &ed : origAdj[node]){
                if(!inMST[ed.first]){
                    pq.push({ed.second, ed.first, node});
                }
            }
        }
        g.setMSTBuilt(totalW);
    }
};

static Graph globalGraph;

// Helper to set MST Algorithm by name
static bool setAlgorithm(const std::string &name) {
    if(name=="KRUSKAL") {
        globalGraph.setAlgorithm(std::make_unique<KruskalMST>());
        return true;
    } else if(name=="PRIM") {
        globalGraph.setAlgorithm(std::make_unique<PrimMST>());
        return true;
    }
    return false;
}

// We'll define a function to handle an *accepted* client socket
static void processClient(int clientFD) {
    const int BUF_SIZE=1024;
    char buffer[BUF_SIZE];
    while(true) {
        memset(buffer,0,BUF_SIZE);
        int n=read(clientFD,buffer,BUF_SIZE-1);
        if(n<=0){
            // client closed or error
            std::cerr<<"[Server] Client disconnected.\n";
            break;
        }
        std::string cmdLine(buffer);
        std::stringstream ss(cmdLine);
        std::string cmd;
        ss >> cmd;
        for(auto &c:cmd) c=toupper(c);

        std::string response;
        if(cmd=="CREATE_GRAPH"){
            int v; ss>>v;
            globalGraph.createGraph(v);
            response="Graph created with "+std::to_string(v)+" vertices.\n";
        }
        else if(cmd=="ADD_EDGE"){
            int u,v; double w; ss>>u>>v>>w;
            globalGraph.addEdge(u,v,w);
            response="Edge added.\n";
        }
        else if(cmd=="SET_ALG"){
            std::string algName; ss>>algName;
            for(auto &c:algName) c=toupper(c);
            if(!setAlgorithm(algName)) {
                response="Unknown MST algorithm.\n";
            } else {
                response="Algorithm set to "+algName+"\n";
            }
        }
        else if(cmd=="BUILD_MST"){
            globalGraph.buildMST();
            if(!globalGraph.isMSTBuilt()) response="Build MST failed.\n";
            else response="MST built.\n";
        }
        else if(cmd=="GET_WEIGHT"){
            double w=globalGraph.getMSTWeight();
            if(w<0) response="MST not built.\n";
            else response="MST Weight = "+std::to_string(w)+"\n";
        }
        else if(cmd=="GET_DIAMETER"){
            if(!globalGraph.isMSTBuilt()) response="MST not built.\n";
            else {
                double d=globalGraph.getLongestDistance();
                response="MST Diameter = "+std::to_string(d)+"\n";
            }
        }
        else if(cmd=="GET_AVG_DIST"){
            if(!globalGraph.isMSTBuilt()) response="MST not built.\n";
            else {
                double ad=globalGraph.getAverageDistance();
                response="MST Avg Dist = "+std::to_string(ad)+"\n";
            }
        }
        else if(cmd=="QUIT" || cmd=="EXIT"){
            response="Goodbye!\n";
            write(clientFD,response.c_str(),response.size());
            break;
        }
        else {
            response="Unknown command.\n";
        }
        if(!response.empty()){
            write(clientFD,response.c_str(), response.size());
        }
    }
    close(clientFD);
}

// ------------------ Leader-Follower Thread Pool ------------------
class LeaderFollowerServer {
private:
    int serverFD;
    int port;
    std::mutex lfMutex;
    std::condition_variable lfCond;
    bool stopFlag;
    int threadCount;
    // The "leader" is the thread currently calling accept().
    // The others are "followers" waiting on the condition variable.

public:
    LeaderFollowerServer(int p, int numThreads)
        : port(p), stopFlag(false), threadCount(numThreads)
    {}

    bool start() {
        serverFD = socket(AF_INET, SOCK_STREAM, 0);
        if(serverFD<0){
            std::cerr<<"[Server] socket() failed\n";
            return false;
        }
        int opt=1;
        setsockopt(serverFD,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
        sockaddr_in servAddr;
        memset(&servAddr,0,sizeof(servAddr));
        servAddr.sin_family=AF_INET;
        servAddr.sin_addr.s_addr=INADDR_ANY;
        servAddr.sin_port=htons(port);
        if(bind(serverFD,(sockaddr*)&servAddr,sizeof(servAddr))<0){
            std::cerr<<"[Server] bind() failed\n";
            close(serverFD);
            return false;
        }
        if(listen(serverFD,5)<0){
            std::cerr<<"[Server] listen() failed\n";
            close(serverFD);
            return false;
        }
        std::cout<<"[Server] Leader-Follower server listening on "<<port<<"\n";

        // Create thread pool
        for(int i=0; i<threadCount; i++){
            std::thread(&LeaderFollowerServer::threadLoop, this).detach();
        }
        return true;
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lk(lfMutex);
            stopFlag = true;   // Set the flag
        }
        lfCond.notify_all();   // Wake up any threads in accept()
        
        if (serverFD >= 0) {   // Close the listening socket if valid
            close(serverFD);
            serverFD = -1;
        }
    }


private:
    // Each thread runs threadLoop: tries to become leader, accept a connection, then processes it.
    void threadLoop() {
        while (true) {
            int clientFD = -1;
            {
                std::unique_lock<std::mutex> lk(lfMutex);
                
                // Check if we are asked to stop before calling accept().
                if (stopFlag) {
                    break;
                }

                sockaddr_in cliAddr;
                socklen_t clilen = sizeof(cliAddr);
                clientFD = accept(serverFD, (sockaddr*)&cliAddr, &clilen);

                if (clientFD < 0) {
                    // accept() failed or was interrupted
                    if (stopFlag) {
                        // If we're stopping, break out
                        break;
                    }
                    // otherwise, keep looping
                    continue;
                }

                // Hand off "leader" role so another thread can accept next
                lfCond.notify_one();
            }

            // Process the client outside the lock
            processClient(clientFD);
        }

    }
};

// ------------------ Main ------------------
int main(){
    const int PORT=12345;
    const int NUM_THREADS=2; // you can increase as needed
    LeaderFollowerServer server(PORT, NUM_THREADS);
    if(!server.start()){
        return 1;
    }

    // The main thread just waits. In a real application,
    // you might do something else or wait for a signal.
    std::cout << "[Server] Press Enter to stop...\n";
    
    // Use getline instead of cin.get() to reliably detect an entire line
    std::string dummy;
    std::getline(std::cin, dummy);

    std::cout << "[Server] Stopping server...\n";
    server.stop();  // triggers stopFlag = true and closes socket
    std::cout << "[Server] Stopped.\n";
    return 0;
}

