#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <random>
#include <fstream>
#include <sstream>


// Simulator 에 활용할 파일을 터미널에 드래그하여 입력받을 때, '' 를 제거해주는 함수
std::string refinePath(std::string fname) {
    std::string path;
    std::remove_copy(fname.begin(), fname.end(), std::back_inserter(path), '\'');
    return path; 
}

// 데이터 관리, RandomWalk 동작, PageRank 기록 및 출력 등 필요한 모든 기능을 갖춘 시뮬레이터
class PRSimulator {
	private:
        typedef std::vector<std::pair<int, int>> edge;
        typedef std::vector<std::pair<double, int>> prob;
        std::map<int, edge> adj; // <시작 노드, 도착 노드, 가중치> 인접 리스트
        std::map<int, std::string> nodeId; // 출력을 위한 nodeId, node name pair
        std::map<int, int> edgeSum;
        std::map<int, prob> probMap;
        std::map<int, int> pageRank;
        bool isDirected;
        bool isWeighted;
    public:
        PRSimulator(): isDirected(false), isWeighted(false) {}
        PRSimulator(bool isDirected, bool isWeighted): isDirected(isDirected), isWeighted(isWeighted) {}
        void loadNode(std::string fname);
        void loadLink(std::string fname);
        void initProb();
        void insertLink(int u, int v, int w);
        int random_0_to_n(int n);
        double random_0_to_1();
        void randomWalk(int here, int n, double q);
        void printAdj();
        void printPageRank(int n);
        void reset();
};


void PRSimulator::loadNode(std::string fname) {
    std::ifstream ifs(fname);
    if(!ifs) {
        std::cout << "file name '" << fname << "' doesn't exist\n";
        exit(1);
    }
    std::string line;
    std::getline(ifs, line);
    std::stringstream ls(line);

    while(!ifs.eof()) {
        int id;
        std::string str;
        std::getline(ifs, line);
        if(line!="") {
            std::stringstream ls(line);
            ls >> id;
            std::string name = "";
            while(ls >> str) {
                if(!name.empty())
                    name += " ";
                name += str;
            }
            nodeId.insert(std::make_pair(id, name));
            edgeSum.insert(std::make_pair(id, 0));
            pageRank.insert(std::make_pair(id, 0));
        }
    }
}

void PRSimulator::loadLink(std::string fname) {
    std::ifstream ifs(fname);
    if(!ifs) {
        std::cout << "file name '" << fname << "' doesn't exist\n";
        exit(1);
    }
    std::string line;
    std::getline(ifs, line);
    std::stringstream ls(line);

    while(!ifs.eof()) {
        int u, v, w;
        std::getline(ifs, line);
        std::stringstream ls(line);
        ls >> u >> v;
        if(isWeighted)
            ls >> w;
        else
            w = 1;
        insertLink(u, v, w);
        edgeSum[u] += w;
        if(!isDirected) {
            insertLink(v, u, w);
            edgeSum[v] += w;
        }
    }
}

void PRSimulator::insertLink(int u, int v, int w) {
    if(adj.find(u) == adj.end())
        adj.insert(std::make_pair(u, edge(1,std::make_pair(v, w))));
    else
        adj[u].push_back(std::make_pair(v, w));
}

void PRSimulator::initProb() {
    for(std::pair<int, std::string> hereId : nodeId) {
        int here = hereId.first;
        probMap.insert(std::make_pair(here, prob()));
        for(std::pair<int, int> therePair: adj[here]) {
            int there = therePair.first;
            int weight = therePair.second;
            double probThere = (double)weight / edgeSum[here];
            probMap[here].push_back(std::make_pair(probThere, there));
        }
        std::partial_sum(probMap[here].begin(), probMap[here].end(), probMap[here].begin(),
            [](const std::pair<double, int>& x, const std::pair<double, int>& y) {return std::pair<double, int>(x.first + y.first, y.second);}
        );
    }
}

int PRSimulator::random_0_to_n(int n){
    static std::mt19937 rand(time(0));
    return rand() % n;
}

double PRSimulator::random_0_to_1() {
    static std::random_device r;
    static std::mt19937 prng(r());
    static std::uniform_real_distribution<> dist(0.0, 1.0);
    return dist(prng);
}

void PRSimulator::randomWalk(int here, int n, double q) {
    int there;

    if(n==0)
        return;

    if(here == -1 || edgeSum[here]==0 || q > random_0_to_1()){
        std::map<int, std::string>::iterator it = nodeId.begin(); 
        std::advance(it, random_0_to_n(nodeId.size()));
        there = it->first;

    } else {
        prob hereProb = probMap[here];
        auto tempProb = std::map<std::pair<double, int>::first_type, std::pair<double, int>::second_type>(hereProb.begin(), hereProb.end());
        there = tempProb.lower_bound(random_0_to_1())->second;

    }
    
    pageRank[there] += 1;
    randomWalk(there, n-1, q);
}

void PRSimulator::printAdj() {
    for(auto pair : nodeId) {
        int here = pair.first; 
        if(!adj[here].empty()) {
            std::cout << nodeId[here] << ": ";
            prob::iterator it = probMap[here].begin();
            for(std::pair<int, int> there: adj[here]) {
                std::cout << nodeId[there.first];
                if(isWeighted) 
                    std::cout << "(" << there.second << ")";
                std::cout << " ";
                std::cout << "[" << it->first << "]" << " ";
                it++;
            }
            std::cout << "// edge sum: " << edgeSum[here];
            std::cout << "\n";
        }
    }
}

void PRSimulator::printPageRank(int n){
    std::priority_queue<std::pair<double, int>> pq;
    // std::cout << "node rank\n";
    for(auto pair: nodeId){
        int id = pair.first;
        double rank = (double)pageRank[id]/n;
        std::string name = pair.second;
        // std::cout << name << " " << (double)pageRank[id]/n << std::endl;
        pq.push(std::make_pair(rank, id));
    }
    // std::cout << "\n";
    std::cout << "node rank (descending)\n";
    int cnt = 10;
    while(!pq.empty() && cnt--)
    {
        std::pair<double, int> pair = pq.top();
        std::string name = nodeId[pair.second];
        std::cout << name << " " << pair.first << std::endl;
        pq.pop();
    }
}

void PRSimulator::reset() {
    pageRank.clear();
}

int main() {
    std::string pathOfNodeFile;
    std::string pathOfLinkFile;
    int isDirected, isWeighted;
    int key = 0, n;
    double q;

    while(key != -1) {
        std::cout << "0. Undirected / 1. Directed\n > "; std::cin >> isDirected;
        std::cout << "0. UnWeighted / 1. Weighted\n > "; std::cin >> isWeighted;

        PRSimulator PRsim = PRSimulator(isDirected, isWeighted);

        std::cout << "nodeFilePath > "; std::cin >> pathOfNodeFile;
        PRsim.loadNode(refinePath(pathOfNodeFile));

        std::cout << "linkFilePath > "; std::cin >> pathOfLinkFile;
        PRsim.loadLink(refinePath(pathOfLinkFile));

        PRsim.initProb();

        key = 0;
        while(key == 0) {
            // q, n 값 받고
            std::cout << "(n, q) > "; std::cin >> n >> q;
            PRsim.randomWalk(-1, n, q);

            // PRsim.printAdj();
            PRsim.printPageRank(n);

            std::cout << "0. rerun / 1. new / -1. exit\n"; 
            std::cout << "> "; std::cin >> key;


            PRsim.reset();
        }
    }

    return 0;
}
