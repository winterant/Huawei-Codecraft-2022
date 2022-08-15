#include <bits/stdc++.h>
using namespace std;
typedef pair<int, int> PII;
const int64_t INF = (1LL << 40);

// ȥ���ַ�����β�հ׷�
string strip(string &str)
{
    auto is_white = [](char ch)
    {
        return ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t';
    };
    int i = 0, j = str.length() - 1;
    while (i <= j && is_white(str[i]))
        ++i;
    while (i <= j && is_white(str[j]))
        --j;
    if (i <= j)
        return str.substr(i, j - i + 1);
    return string();
}

// ��ȡ�����ļ�config.ini
string getConfigIni(string filepath, string appName, string keyName, string defaultValue = string())
{
    ifstream fin(filepath);
    if (!fin)
    {
        cout << "No such a file " << filepath << endl;
        exit(1);
    }
    string curAppName = string(), line, retValue = defaultValue;
    while (getline(fin, line))
    {
        line = strip(line);
        if (line.length() > 2 && line[0] == '[' && line.back() == ']')
            curAppName = line.substr(1, line.length() - 2);
        if (curAppName == appName)
        {
            int eq_pos = line.find('=');
            if (eq_pos != -1 && line.substr(0, eq_pos) == keyName) //�ҵ�������
                return line.substr(eq_pos + 1);
        }
    }
    return retValue;
}

// ��Ե�ڵ��
class SiteList
{
private:
    vector<string> siteNames;   //��Ե�ڵ������б�
    map<string, int> siteId;    //��Ե�ڵ��������
    vector<int> siteBandwidths; //��Ե�ڵ�������ޱ�

public:
    explicit SiteList(string filepath) //��ȡ�ļ����췽��
    {
        ifstream fin(filepath, ios::in);
        if (!fin)
        {
            cout << "Can't open file " << filepath << endl;
            exit(3);
        }
        string line, siteName, bandwidth;
        getline(fin, line); // ��ȡ��ͷ
        while (getline(fin, line))
        {
            istringstream sin(strip(line));
            getline(sin, siteName, ',');  // ��ȡ�ڵ���
            getline(sin, bandwidth, ','); // ��ȡ��������
            siteNames.push_back(siteName);
            siteBandwidths.push_back(atoi(bandwidth.c_str()));
        }
        fin.close();
        for (int i = 0; i < (int)siteNames.size(); ++i)
            siteId[siteNames[i]] = i;
    }

    int getNumSites()
    {
        return siteNames.size();
    }

    string &getSiteName(int siteId)
    {
        return siteNames[siteId];
    }

    int getSiteId(string &siteName)
    {
        return siteId[siteName];
    }

    int getBandwidth(int siteId)
    {
        return siteBandwidths[siteId];
    }
};

// �û������
class Demand
{
private:
    vector<string> moments;     // ʱ�̱�
    vector<string> userNames;   // �û�����
    map<string, int> userId;    // �û������û����
    vector<vector<int>> demand; // �����

public:
    explicit Demand(string filepath)
    {
        ifstream fin(filepath, ios::in);
        if (!fin)
        {
            cout << "Can't open file " << filepath << endl;
            exit(3);
        }
        string line, username, moment, bandwidth;
        // ��ȡ��ͷ
        getline(fin, line);
        istringstream sin(strip(line));
        getline(sin, username, ','); // mtime
        while (getline(sin, username, ','))
            userNames.push_back(strip(username));
        for (int i = 0; i < (int)userNames.size(); ++i)
            userId[userNames[i]] = i;
        // ��ȡ����ֵ
        for (int t = 0; getline(fin, line); t++) // t����ʱ��
        {
            istringstream sin(strip(line));
            getline(sin, moment, ',');
            moments.push_back(moment);
            vector<int> dem; //��ǰʱ�̵��û�������
            while (getline(sin, bandwidth, ','))
            {
                dem.push_back(atoi(bandwidth.c_str()));
            }
            demand.push_back(dem); // tʱ�̵���������
        }
        fin.close();
    }

    int getDemand(int moment, int user)
    {
        return demand[moment][user];
    }

    int getNumMoments()
    {
        return moments.size();
    }

    int getNumUsers()
    {
        return userNames.size();
    }

    string &getUserName(int userId)
    {
        return userNames[userId];
    }

    int getUserId(string &userName)
    {
        return userId[userName];
    }
};

// ʱ�ӱ�
class Qos
{
private:
    vector<vector<int>> qos;

public:
    Qos(string filepath, SiteList &siteList, Demand &demand)
    {
        ifstream fin(filepath, ios::in);
        if (!fin)
        {
            cout << "Can't open file " << filepath << endl;
            exit(3);
        }
        // ��ʼ��qos��С
        qos.resize(siteList.getNumSites());
        for (auto &e : qos)
            e.resize(demand.getNumUsers());

        /******** ��ȡ���� ***********/
        string line, siteName, username, q;
        // ��ȡ��һ���û�����˳��ӳ��Ϊ���
        getline(fin, line);
        istringstream sin(strip(line));
        getline(sin, username, ',');
        vector<int> userIds(demand.getNumUsers());
        for (int u = 0; getline(sin, username, ','); ++u)
        {
            userIds[u] = demand.getUserId(username); //��ȡ�û����
        }
        // ��ȡʱ�Ӿ���qos
        for (int t = 0; getline(fin, line); t++) // t����ʱ��
        {
            istringstream sin(strip(line));
            getline(sin, siteName, ',');
            int siteId = siteList.getSiteId(siteName); //��ȡ��Ե�ڵ���
            for (int u = 0; getline(sin, q, ','); ++u)
                qos[siteId][userIds[u]] = atoi(strip(q).c_str());
        }
        fin.close();
    }

    int get(int siteId, int userId)
    {
        return qos[siteId][userId];
    }
};

// �������
class Solution
{
private:
    // ���ݼ�
    SiteList &siteList; //��Ե�ڵ㼯
    Demand &demand;     //����
    Qos &qos;           //ʱ�ӱ�
    int qosConstraint;  //ʱ������

    // ���ݼ�����
    int numMoments;   //ʱ������
    int numSites;     //��Ե�ڵ�����
    int numUsers;     //�û�����
    int num95moments; // 95�ٷ�λʱ������
    int num5moments;  // 5�ٷ�λʱ������

    // ��¼ʵʱ����
    vector<vector<vector<int>>> userFlow; // userFlow[t][s][u]��ʾtʱ�̽ڵ�s���û�u֮�������
    vector<vector<int>> siteFlow;         // siteFlow[t][s]��ʾtʱ��s�ڵ��ϵ�������

    // ͼ
    struct Edge
    {
        int from, to;
        int64_t cost;
        int64_t cap, flow;
        bool isForward;
        PII reversedEdge; // <node id, edge index>
        Edge(int from, int to, int64_t cost, int64_t cap, int64_t flow, bool isForward)
        {
            this->from = from;
            this->to = to;
            this->cost = cost;
            this->cap = cap;
            this->flow = flow;
            this->isForward = isForward;
        }
        // Ϊͼ�Ͻڵ��ŷ���һ�����
        static int assginId()
        {
            static int nodeId = 0;
            return nodeId++;
        }
    };
    // ʹ���ڽӱ���ͼ��������ԭ��ڵ���ͼ�Ͻڵ�Ķ�Ӧ��ϵ
    int superSource, superDest;         // ����Դ�㣬�������
    vector<vector<Edge>> graph;         // �ڽӱ�
    vector<vector<int>> userId2GraphId; // [t][u] �û��ڵ�u��ͼ���
    vector<vector<int>> siteId2GraphId; // [t][s] tʱ�̱�Ե�ڵ�s��ͼ���
    vector<PII> graphId2ServeId;         // ͼ�Ͻڵ�i����Ӧ����ʵ�ı�Ե�ڵ��±�[t][u]

public:
    Solution(SiteList &siteList, Demand &demand, Qos &qos, int qosConstraint)
        : siteList(siteList), demand(demand), qos(qos), qosConstraint(qosConstraint)
    {
        // ��ȡ���ݼ�����
        numMoments = demand.getNumMoments();
        numSites = siteList.getNumSites();
        numUsers = demand.getNumUsers();
        num5moments = numMoments / 20;
        num95moments = numMoments - num5moments;
        // ��ʼ��userFlow
        userFlow.resize(numMoments);
        for (auto &su : userFlow)
        {
            su.resize(numSites);
            for (auto &users : su)
                users.resize(numUsers, 0);
        }
        // ��ʼ��siteFlow
        siteFlow.resize(numMoments);
        for (auto &sites : siteFlow)
            sites.resize(numSites, 0);
        // ��ʼ��ͼ
        graph.resize(2 + numMoments * (numUsers + numSites)); // ͼ�Ͻڵ�ĸ�����2��Դ��㣬user/site������T��
        userId2GraphId.resize(numMoments);
        siteId2GraphId.resize(numMoments);
        for (int t = 0; t < numMoments; ++t)
        {
            userId2GraphId[t].resize(numUsers);
            siteId2GraphId[t].resize(numSites);
        }
        graphId2ServeId.resize(graph.size());
    }

    // ���ڽӱ�����ӱ�
    void addEdge(int from, int to, int64_t cost, int64_t cap) //���from->to�����
    {
        graph[from].push_back(Edge(from, to, cost, cap, 0, true));
        graph[to].push_back(Edge(to, from, -cost, 0, 0, false)); //���ڱ�
        graph[from].back().reversedEdge = PII(to, graph[to].size() - 1);
        graph[to].back().reversedEdge = PII(from, graph[from].size() - 1);
    }

    // ��ȡ�����
    Edge &getReversedEdge(Edge &edge)
    {
        return graph[edge.reversedEdge.first][edge.reversedEdge.second];
    }

    // ����ĳһ���ߵĴ��ۺ�����
    void resetCostOfEdge(Edge &edge, int64_t cost, int64_t cap)
    {
        edge.cost = cost;
        getReversedEdge(edge).cost = -cost;
        edge.cap = cap;
    }

    // ��ͼ
    void buildGraph(int64_t defaultCost = 1)
    {
        // ������
        superSource = Edge::assginId();
        superDest = Edge::assginId();
        for (int t = 0; t < numMoments; ++t)
            for (int u = 0; u < numUsers; ++u)
                userId2GraphId[t][u] = Edge::assginId();
        for (int t = 0; t < numMoments; ++t)
            for (int s = 0; s < numSites; ++s)
            {
                siteId2GraphId[t][s] = Edge::assginId();
                graphId2ServeId[siteId2GraphId[t][s]] = PII(t, s);
            }

        // ��ͼ
        for (int t = 0; t < numMoments; ++t)
        {
            // ����Դ��->user������1������=demand
            for (int u = 0; u < numUsers; ++u)
                addEdge(superSource, userId2GraphId[t][u], defaultCost, demand.getDemand(t, u));
            // user->site������1������=INF
            for (int s = 0; s < numSites; ++s)
            {
                for (int u = 0; u < numUsers; ++u)
                    if (qos.get(s, u) < qosConstraint)
                        addEdge(userId2GraphId[t][u], siteId2GraphId[t][s], defaultCost, INF);
            }
            // site->��㣬��ߣ�Ĭ�ϴ���1������֮��=����
            for (int s = 0; s < numSites; ++s)
            {
                int bandwidth = siteList.getBandwidth(s);
                addEdge(siteId2GraphId[t][s], superDest, defaultCost * 1, bandwidth / 4);
                addEdge(siteId2GraphId[t][s], superDest, defaultCost * 2, bandwidth / 4);
                addEdge(siteId2GraphId[t][s], superDest, defaultCost * 3, bandwidth / 4);
                addEdge(siteId2GraphId[t][s], superDest, defaultCost * 4, bandwidth - bandwidth / 4 * 3);
            }
        }
        cout << "Meked Graph with number of node: " << graph.size() << endl;
    }

    // ����ָ����Ե�ڵ㵽�������Ĵ��ۣ����ڱߴ���ȡ��
    void reassignCostOfSite()
    {
        // ����ÿ���ڵ㣬����ָ������
        for (int s = 0; s < numSites; ++s)
        {
            // ͳ�������������
            static vector<int64_t> flows(numMoments);
            static vector<int64_t> sortedFlows(numMoments);
            for (int t = 0; t < numMoments; ++t)
            {
                flows[t] = sortedFlows[t] = 0;
                for (auto &edge : graph[siteId2GraphId[t][s]])
                {
                    if (edge.isForward && edge.flow > 0)
                    {
                        flows[t] += edge.flow;
                        sortedFlows[t] += edge.flow;
                    }
                }
            }
            sort(sortedFlows.begin(), sortedFlows.end());
            vector<int64_t> threshold({0,
                                       (int64_t)(sortedFlows[num95moments - 1] * 0.9),
                                       sortedFlows[num95moments - 1],
                                       sortedFlows[num95moments],
                                       siteList.getBandwidth(s)}); //��ֵ
            // �޸ıߴ���
            for (int t = 0; t < numMoments; ++t)
            {
                int level = 0;
                for (Edge &edge : graph[siteId2GraphId[t][s]])
                    if (edge.isForward)
                    {
                        ++level;
                        if (flows[t] <= sortedFlows[num95moments]) // С��96; ��4����
                            resetCostOfEdge(edge, level, threshold[level] - threshold[level - 1]);
                        else // ���4��ֱ�����أ�����Ϊ1
                            resetCostOfEdge(edge, 1, threshold[level] - threshold[level - 1]);
                    }
            }
        }
        // ͼ��������
        for (auto &edges : graph)
            for (auto &e : edges)
                e.flow = 0;
    }

    // spfa�㷨������Լ���µ����·
    bool spfa(int source, int dest, vector<int64_t> &cost, vector<int> &level, vector<Edge *> &preEdge)
    {
        static vector<bool> inQ(graph.size());
        for (uint32_t i = 0; i < graph.size(); ++i)
        {
            cost[i] = INF;
            inQ[i] = false;
        }
        cost[source] = 0;
        level[source] = 0;
        inQ[source] = true;
        queue<int> Q;
        Q.push(source);
        while (!Q.empty())
        {
            int u = Q.front();
            Q.pop();
            inQ[u] = false;
            for (auto &e : graph[u])
            {
                if (e.cap - e.flow > 0 && cost[e.to] > cost[u] + e.cost) // ֻҪ������ && ·�����
                {
                    level[e.to] = level[e.from] + 1;
                    cost[e.to] = cost[u] + e.cost;
                    preEdge[e.to] = &e; //��סǰ���ߵĵ�ַ
                    if (!inQ[e.to])
                    {
                        Q.push(e.to); //��������˫�˶����Ż�һ�£�����С�����ȷ��ڶ���
                        inQ[e.to] = true;
                    }
                }
            }
        }
        return cost[dest] < INF;
    }

    // �������������
    int64_t dfs(int curNode, int dest, vector<int64_t> &cost, vector<int> &level, int64_t curFlowLimit, int64_t &totalCost)
    {
        if (curNode == dest)
            return curFlowLimit;
        int64_t sonsFlowLimit = 0;
        for (auto &edge : graph[curNode])
        {
            if (curFlowLimit == sonsFlowLimit)
                break;
            if (cost[curNode] + edge.cost == cost[edge.to] && level[curNode] + 1 == level[edge.to] && edge.cap - edge.flow > 0)
            {
                int64_t sonFlow = dfs(edge.to, dest, cost, level, min(curFlowLimit - sonsFlowLimit, edge.cap - edge.flow), totalCost);

                sonsFlowLimit += sonFlow;
                totalCost += sonFlow * edge.cost;

                edge.flow += sonFlow;
                getReversedEdge(edge).flow -= sonFlow;
            }
        }
        return sonsFlowLimit;
    }

    // zkw�㷨�����С������
    int64_t mincost(int source, int dest)
    {
        cout << "Start to find augmenting path." << endl;
        static vector<int64_t> cost(graph.size());   // ��ǰ��Ļ���
        static vector<int64_t> flow(graph.size());   // ��ǰ�������
        static vector<int> level(graph.size());      //��ǰ������·�㼶
        static vector<Edge *> preEdge(graph.size()); // ��ǰ���ǰ���ߣ�ָ�룩
        int64_t totalCost = 0;                       // �ܻ���
        int64_t totalFlow = 0;                       // ������
        while (spfa(source, dest, cost, level, preEdge))
        {
            int64_t getFlow = 1;
            while (getFlow > 0)
            {
                getFlow = dfs(source, dest, cost, level, INF, totalCost);
                totalFlow += getFlow;
            }
        }
        cout << "Total flow " << totalFlow << " and total cost " << totalCost << endl;
        return totalCost;
    }

    // ͳ��ͼ�ϵ�����
    void gatherFlowOfGraph()
    {
        for (int t = 0; t < numMoments; ++t)
        {
            for (int u = 0; u < numUsers; ++u)
            {
                for (auto &e : graph[userId2GraphId[t][u]])
                {
                    if (e.isForward && e.flow < 0)
                    {
                        cout << "ERROR: ���������Ϊ����" << endl;
                        exit(-2);
                    }
                    if (e.isForward && e.flow > 0)
                    {
                        int moment = graphId2ServeId[e.to].first;
                        int siteId = graphId2ServeId[e.to].second;
                        userFlow[moment][siteId][u] += e.flow;
                        siteFlow[moment][siteId] += e.flow;
                    }
                }
            }
        }
    }

    // �����ܳɱ�
    int64_t calclateProblemCost()
    {
        int64_t cost = 0;
        for (int s = 0; s < numSites; ++s)
        {
            vector<int64_t> flows(numMoments);
            for (int t = 0; t < numMoments; ++t)
                flows[t] = siteFlow[t][s];
            sort(flows.begin(), flows.end());
            cost += flows[num95moments - 1];
        }
        return cost;
    }

    // ����������ļ�
    void output(string filepath)
    {
        cout << "Start to output solution." << endl;
        ofstream fout(filepath, ios::out);
        for (int t = 0; t < numMoments; ++t)
        {
            //����ÿһ��ʱ�̣������û�
            for (int u = 0; u < numUsers; ++u)
            {
                //����tʱ�̵��û�u����������ӵ�������������0�ı�Ե�ڵ�
                fout << demand.getUserName(u) << ":";
                bool first = true;
                for (int s = 0; s < numSites; ++s)
                {
                    if (userFlow[t][s][u] > 0)
                    {
                        if (first)
                            first = false;
                        else
                            fout << ',';
                        fout << '<' << siteList.getSiteName(s) << ',' << userFlow[t][s][u] << '>';
                    }
                }
                fout << endl;
            }
        }
        fout.close();
        cout << "Solution has been wrote to file " << filepath << endl;
        cout << "Final cost: " << calclateProblemCost() << endl;
    }

    void run()
    {
        buildGraph();
        mincost(superSource, superDest); // ��ʼ��һ����
        for (int epoch = 0; epoch < 5; epoch++)
        {
            reassignCostOfSite(); // �Ƚ��������㣬�����ô���
            mincost(superSource, superDest);
        }
        gatherFlowOfGraph();
    }
};

int main()
{
    /******** ȷ�����ݼ�·�� ********/
    string dataDir = "/data", outputPath = "/output/solution.txt"; // Ĭ�����ϻ���
    if (ifstream("../data/config.ini").good())                     // windows���Ի���
    {
        // dataDir = "../data";
        // outputPath = "../data/solution.txt";
        dataDir = "../pressure0.6_data";
        outputPath = "../pressure0.6_data/solution.txt";
    }
    else if (ifstream("../CodeCraft-2022/data/config.ini").good()) // linux���Ի���
    {
        dataDir = "../CodeCraft-2022/data";
        outputPath = "../CodeCraft-2022/data/solution.txt";
    }

    /********* ��ȡ���ݼ� *********/
    SiteList siteList(dataDir + "/site_bandwidth.csv");
    Demand demand(dataDir + "/demand.csv");
    Qos qos(dataDir + "/qos.csv", siteList, demand);
    int qosConstraint = atoi(getConfigIni(dataDir + "/config.ini", "config", "qos_constraint").c_str());

    /********* ִ�н������ *********/
    Solution solution(siteList, demand, qos, qosConstraint);
    solution.run();

    /********* �����д���ļ� *********/
    solution.output(outputPath);
    return 0;
}
