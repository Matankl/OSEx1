/**************************************************************
 * server_pipeline.cpp
 * Demonstrates a simple Pipeline pattern with an Active Object.
 *
 * Steps:
 *  1) Main thread (Network I/O) accepts connections, reads requests,
 *     and enqueues them into a "task queue."
 *  2) The ActiveObject runs in its own thread, continuously dequeuing
 *     tasks and processing MST commands.
 *  3) The result is delivered back to the network layer, which sends
 *     it to the client.
 *
 * For simplicity:
 *  - We handle a single client connection at a time in this example.
 *  - The concurrency focus is on pipeline + active object pattern,
 *    not multi-client concurrency. You can extend it for multiple
 *    clients by repeating the same approach in multiple threads or
 *    using nonblocking I/O.
 ***************************************************************/

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <memory>
#include <atomic>
#include <unistd.h>   // for close(), read(), write()
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>    // for memset

//----------------------------------------------------------
// MST & Graph classes (same logic as in previous examples)
//----------------------------------------------------------

// Forward declarations
class Graph;
class MSTAlgorithm {
public:
    virtual ~MSTAlgorithm() = default;
    virtual void buildMST(Graph& graph) = 0;
};

// ========== Graph Class ==========
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

//----------------------------------------------------------
// Active Object & Pipeline Implementation
//----------------------------------------------------------

// A "Task" is something we want the ActiveObject to process:
//   - Contains the command string from the client
//   - A way to store the response (string)
struct Task {
    std::string command;       // e.g. "ADD_EDGE 0 1 2.0"
    std::string result;        // the processed result (server response)
    int clientSocket;          // where we send result
};

// ActiveObject class: runs in its own thread, processes tasks from a queue
class ActiveObject {
private:
    std::thread worker;
    std::mutex qMutex;
    std::condition_variable qCV;
    std::queue<std::shared_ptr<Task>> taskQueue;
    std::atomic<bool> stopFlag{false};

public:
    // We'll store references to the *shared* MST "Graph" here:
    Graph &graph;

    ActiveObject(Graph &g) : graph(g) {
        // Launch worker thread
        worker = std::thread(&ActiveObject::run, this);
    }

    ~ActiveObject() {
        // Signal stop and join worker
        stopFlag = true;
        qCV.notify_all();
        if(worker.joinable()) {
            worker.join();
        }
    }

    // Enqueue a task
    void submit(std::shared_ptr<Task> task) {
        {
            std::lock_guard<std::mutex> lk(qMutex);
            taskQueue.push(task);
        }
        qCV.notify_one();
    }

private:
    // Worker thread function
    void run() {
        while(!stopFlag) {
            std::shared_ptr<Task> currentTask;
            {
                std::unique_lock<std::mutex> lk(qMutex);
                qCV.wait(lk, [this]{ return !taskQueue.empty() || stopFlag; });
                if(stopFlag && taskQueue.empty()) {
                    break; 
                }
                currentTask = taskQueue.front();
                taskQueue.pop();
            }
            // Process the task
            process(currentTask);
        }
    }

    // Actual MST command processing
    void process(std::shared_ptr<Task> task) {
        // parse the command
        std::stringstream ss(task->command);
        std::string cmd;
        ss >> cmd;
        // uppercase
        for (auto &c : cmd) { c = toupper(c); }

        std::string response;

        if(cmd=="CREATE_GRAPH"){
            int v; ss >> v;
            graph.createGraph(v);
            response = "Graph created with " + std::to_string(v) + " vertices.\n";
        }
        else if(cmd=="ADD_EDGE"){
            int u,v; double w; 
            ss >> u >> v >> w;
            graph.addEdge(u,v,w);
            response = "Edge added (" + std::to_string(u)+","+std::to_string(v)+"), w="+std::to_string(w)+"\n";
        }
        else if(cmd=="SET_ALG"){
            std::string algName; ss >> algName;
            for(auto &c: algName){ c=toupper(c); }
            if(algName=="KRUSKAL"){
                graph.setAlgorithm(std::make_unique<KruskalMST>());
                response = "Algorithm set to KRUSKAL\n";
            } else if(algName=="PRIM"){
                graph.setAlgorithm(std::make_unique<PrimMST>());
                response = "Algorithm set to PRIM\n";
            } else {
                response = "Unknown algorithm: " + algName + "\n";
            }
        }
        else if(cmd=="BUILD_MST"){
            graph.buildMST();
            if(!graph.isMSTBuilt()){
                response = "Failed to build MST. Possibly no algorithm set.\n";
            } else {
                response = "MST built successfully.\n";
            }
        }
        else if(cmd=="GET_WEIGHT"){
            double w=graph.getMSTWeight();
            if(w<0) response="MST not built yet.\n";
            else response="MST Weight = " + std::to_string(w) + "\n";
        }
        else if(cmd=="GET_DIAMETER"){
            if(!graph.isMSTBuilt()) response="MST not built yet.\n";
            else {
                double d=graph.getLongestDistance();
                response = "MST Diameter = " + std::to_string(d) + "\n";
            }
        }
        else if(cmd=="GET_AVG_DIST"){
            if(!graph.isMSTBuilt()) response="MST not built yet.\n";
            else {
                double avg=graph.getAverageDistance();
                response = "MST Average Distance = " + std::to_string(avg) + "\n";
            }
        }
        else if(cmd=="QUIT" || cmd=="EXIT"){
            response = "Goodbye!\n";
        }
        else {
            response = "Unknown command: " + cmd + "\n";
        }

        task->result = response;

        // We could send the response directly here, or pass it to another pipeline stage.
        // For simplicity, let's send it back now:
        write(task->clientSocket, response.c_str(), response.size());
    }
};

//----------------------------------------------------------
// The Pipeline-Style Server
//----------------------------------------------------------
static const int PORT = 12345;

int main() {
    // We'll have a single global Graph
    static Graph globalGraph;

    // ActiveObject to process MST tasks
    static ActiveObject activeObj(globalGraph);

    // Socket setup
    int serverFD = socket(AF_INET, SOCK_STREAM, 0);
    if(serverFD<0){
        std::cerr<<"[Server] socket() failed\n";
        return 1;
    }

    int opt=1;
    setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in servAddr;
    memset(&servAddr,0,sizeof(servAddr));
    servAddr.sin_family=AF_INET;
    servAddr.sin_addr.s_addr=INADDR_ANY;
    servAddr.sin_port=htons(PORT);

    if(bind(serverFD,(sockaddr*)&servAddr,sizeof(servAddr))<0){
        std::cerr<<"[Server] bind() failed\n";
        close(serverFD);
        return 1;
    }
    if(listen(serverFD,1)<0){
        std::cerr<<"[Server] listen() failed\n";
        close(serverFD);
        return 1;
    }

    std::cout<<"[Server] Pipeline/ActiveObject server listening on port "<<PORT<<"...\n";

    // Accept one client for demonstration
    sockaddr_in clientAddr;
    socklen_t clientLen=sizeof(clientAddr);
    int clientFD=accept(serverFD,(sockaddr*)&clientAddr,&clientLen);
    if(clientFD<0){
        std::cerr<<"[Server] accept() failed\n";
        close(serverFD);
        return 1;
    }

    std::cout<<"[Server] Client connected.\n";

    // Read lines from client, create tasks, submit to ActiveObject
    const int BUF_SIZE=1024;
    char buffer[BUF_SIZE];
    while(true){
        memset(buffer,0,BUF_SIZE);
        int n=read(clientFD, buffer, BUF_SIZE-1);
        if(n<=0){
            std::cerr<<"[Server] Client disconnected.\n";
            break;
        }
        std::string cmdLine(buffer);
        // We check if the user typed QUIT/EXIT => we'll handle that as well
        // We'll still pass to the active object but also break after.

        // Create a Task
        auto t = std::make_shared<Task>();
        t->command      = cmdLine;
        t->clientSocket = clientFD;

        // Submit it to the active object
        activeObj.submit(t);

        // If the command was QUIT/EXIT, let's close
        // (We *could* wait for the active object to respond first, which it will do.)
        if(cmdLine.find("QUIT")==0 || cmdLine.find("EXIT")==0){
            // We'll just break after sending
            break;
        }
    }

    close(clientFD);
    close(serverFD);
    return 0;
}
