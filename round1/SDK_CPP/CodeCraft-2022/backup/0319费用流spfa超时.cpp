#include <bits/stdc++.h>
using namespace std;
typedef pair<int, int> PII;
typedef long long LL;
const LL INF = (1LL << 50);

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
public:
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
    struct EKEdge
    {
        int from, to;
        LL cost, cap, flow;
        bool isForward;
        PII reversedEdge; //<node,index>
        EKEdge(int from, int to, LL cost, LL cap, LL flow, bool isForward)
        {
            this->from = from;
            this->to = to;
            this->cost = cost;
            this->cap = cap;
            this->flow = flow;
            this->isForward = isForward;
        }
    };
    int superSource, superDest;         // ����Դ��, �������
    vector<vector<EKEdge>> graph;       // �ڽӱ�
    vector<vector<int>> siteId2GraphId; // [t][s] tʱ�̱�Ե�ڵ�s��ͼ���
    vector<vector<int>> userId2GraphId; // [t][u] tʱ���û��ڵ�u��ͼ���
    vector<PII> graphId2ServeId;         // ͼ�Ͻڵ�i����Ӧ����ʵ�ı�Ե�ڵ��±�[t][u]

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
        for (auto &i : userFlow)
        {
            i.resize(numSites);
            for (auto &j : i)
            {
                j.resize(numUsers);
                for (auto &e : j)
                    e = 0; // ��ʼ��ÿ���ͻ�����Ϊ0
            }
        }
        // ��ʼ��siteFlow
        siteFlow.resize(numMoments);
        for (auto &i : siteFlow)
        {
            i.resize(numSites);
            for (auto &j : i)
                j = 0; // ��ʼ��ÿ���ڵ�������Ϊ0
        }
        // ��ʼ��ͼ
        superSource = 0;
        superDest = 1;
        graph.resize(numMoments * (numUsers + numSites) + 2); // 0Դ�� 1���
        siteId2GraphId.resize(numMoments);
        userId2GraphId.resize(numMoments);
        for (int t = 0; t < numMoments; ++t)
        {
            siteId2GraphId[t].resize(numSites);
            userId2GraphId[t].resize(numUsers);
        }
        graphId2ServeId.resize(graph.size());
    }

    // ���ڽӱ�����ӱ�
    void addEKEdge(int from, int to, LL cost, LL cap) //���from->to�����
    {
        graph[from].push_back(EKEdge(from, to, cost, cap, 0, true));
        graph[to].push_back(EKEdge(to, from, -cost, 0, 0, false)); //���ڱ�
        graph[from].back().reversedEdge = PII(to, graph[to].size() - 1);
        graph[to].back().reversedEdge = PII(from, graph[from].size() - 1);
    }

    // ��ͼ
    void makeEKGraph()
    {
        // ������
        int nodeId = 1;
        for (int t = 0; t < numMoments; ++t)
        {
            for (int s = 0; s < numSites; ++s)
            {
                siteId2GraphId[t][s] = ++nodeId;
                graphId2ServeId[nodeId] = PII(t, s);
            }
            for (int u = 0; u < numUsers; ++u)
                userId2GraphId[t][u] = ++nodeId;
        }
        // ��ͼ
        for (int t = 0; t < numMoments; ++t)
        {
            // user->site����,����0����������
            for (int s = 0; s < numSites; ++s)
            {
                for (int u = 0; u < numUsers; ++u)
                {
                    if (qos.get(s, u) >= qosConstraint)
                        continue;
                    addEKEdge(userId2GraphId[t][u], siteId2GraphId[t][s], 0, INF);
                }
            }
            // site-������㣬����1������=����
            for (int s = 0; s < numSites; ++s)
                addEKEdge(siteId2GraphId[t][s], superDest, 1, siteList.getBandwidth(s));
            // ����Դ��-user������0������=����
            for (int u = 0; u < numUsers; ++u)
                addEKEdge(superSource, userId2GraphId[t][u], 0, demand.getDemand(t, u));
        }
        cout << "Meked Graph with number of node: " << graph.size() << endl;
    }

    // spfa�㷨������·
    bool spfa(int source, int dest, vector<LL> &cost, vector<LL> &flow, vector<EKEdge *> &preEdge)
    {
        static vector<bool> inQ(graph.size());
        for (int i = 0; i < (int)graph.size(); ++i)
        {
            cost[i] = INF;
            inQ[i] = false;
        }
        cost[source] = 0;
        flow[source] = INF;
        queue<int> Q;
        Q.push(source);
        while (!Q.empty())
        {
            int u = Q.front();
            Q.pop();
            inQ[u] = 0;
            for (auto &e : graph[u])
            {
                if (e.cap - e.flow > 0 && cost[e.to] > cost[u] + e.cost) // ֻҪ������������·�����
                {
                    flow[e.to] = min(flow[u], e.cap - e.flow);
                    cost[e.to] = cost[u] + e.cost;
                    preEdge[e.to] = &e; //��סǰ���ߵĵ�ַ
                    if (!inQ[e.to])
                    {
                        Q.push(e.to);
                        inQ[e.to] = 1;
                    }
                }
            }
        }
        return cost[dest] < INF;
    }

    LL mincost(int source, int dest)
    {
        cout << "Start to find augmenting path." << endl;
        static vector<LL> cost(graph.size()), flow(graph.size()); // ��ǰ��Ļ��Ѻ�����
        static vector<EKEdge *> preEdge(graph.size());            // ��ǰ���ǰ���ߣ�ָ�룩
        LL totalFlow = 0, totalCost = 0;                   // ���������ܻ���
        while (spfa(source, dest, cost, flow, preEdge))
        {
            // cout << " -- Found a augmenting path with flow " << flow[dest] << " and cost " << cost[dest] << endl;
            totalFlow += flow[dest];              // ��������·������
            totalCost += cost[dest] * flow[dest]; // ����
            for (int p = dest; p != source; p = preEdge[p]->from)
            {
                preEdge[p]->flow += flow[dest];
                graph[preEdge[p]->reversedEdge.first][preEdge[p]->reversedEdge.second].flow -= flow[dest];
            }
        }
        // ͳ������
        cout << "Start to gather flow with total flow " << totalFlow << " and total cost " << totalCost << endl;
        for (int t = 0; t < numMoments; ++t)
        {
            for (int u = 0; u < numUsers; ++u)
            {
                for (auto &e : graph[userId2GraphId[t][u]]) // user���, ��ָ���site��Ȼ��ͬʱ�̵�
                {
                    if (e.isForward && e.flow)
                    {
                        int siteId = graphId2ServeId[e.to].second;
                        // cout << "moment " << t << " user " << u << " -> site " << siteId << "  Flow=" << e.flow << " " << graph[e.reversedEdge.first][e.reversedEdge.second].flow << endl;
                        userFlow[t][siteId][u] += e.flow; // + graph[e.reversedEdge.first][e.reversedEdge.second].flow;
                    }
                }
            }
        }
        cout << "Gather flow end." << endl;
        return totalCost;
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
                    if (userFlow[t][s][u] == 0)
                        continue;
                    if (first)
                        first = false;
                    else
                        fout << ',';
                    fout << '<' << siteList.getSiteName(s) << ',' << userFlow[t][s][u] << '>';
                }
                fout << endl;
            }
        }
        fout.close();
        cout << "Solution has been wrote to file " << filepath << endl;
    }

    int run()
    {
        makeEKGraph();
        mincost(superSource, superDest);
        return 0;
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
    int cost = solution.run();
    cout << "Final Cost: " << cost << endl;

    /********* �����д���ļ� *********/
    solution.output(outputPath);
    return 0;
}
