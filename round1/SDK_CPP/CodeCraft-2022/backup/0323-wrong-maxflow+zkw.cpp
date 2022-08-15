#include <bits/stdc++.h>
using namespace std;
typedef pair<int, int> PII;
typedef pair<int64_t, int> PLI;
typedef pair<int64_t, int64_t> PLL;
const int64_t INF = (1LL << 40);
bool DEBUG = true; // ����Ƿ�����

// ����ȡ���ĳ���
int64_t upperDiv(const int64_t &num, const int64_t &div)
{
    return (num + div - 1) / div;
}

// pair�ӷ�
PLL PLLAdd(const PLL &a, const PLL &b)
{
    return PLL(a.first + b.first, a.second + b.second);
}

// pair����
PLL PLLMinus(const PLL &a, const PLL &b)
{
    return PLL(a.first - b.first, a.second - b.second);
}

PLL PLLMin(const PLL &a, const PLL &b)
{
    return PLL(min(a.first, b.first), min(a.second, b.second));
}
// ȥ���ַ�����β�հ׷�
string strip(const string &str)
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
    /********************** ���ݼ���Ϣ���� *****************************/
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

    // ���Ԥ����洢
    vector<vector<int>> siteConnUserIds; // ÿ����Ե�ڵ�����ӵ��û�����
    vector<vector<int>> userConnSiteIds; // ÿ���û������ӵı�Ե�ڵ㼯��
    vector<int> sortedSiteIds;           // ����ı�Ե�ڵ��ţ������ӵĿ�ɾ��
    vector<int> sortedUserIds;           // ������û���ţ������ӵĿ�ɾ��

    /****************************** ̰�Ĳ��� *****************************/
    vector<vector<bool>> is5; // ��ǵ�ǰ�ǲ���5%�ڵ�

    /******************************** ͼ *******************************/
    struct Edge
    {
        // �ڵ���
        int from, to;
        // ���������,������5%
        PLL cap5;  // <cap1��������Ϊ5%, cap2��������>
        PLL flow5; // <flow1����, flow2����>
        // ����������,���ڹ滮ȫ��
        int64_t cost;
        int64_t cap, flow;
        // ����Ϣ
        bool isForward;
        PII reversedEdge; // <node id, edge index>
        Edge(int from, int to, PLL cap5, PLL flow5, int64_t cost, int64_t cap, int64_t flow, bool isForward)
            : from(from), to(to), cap5(cap5), flow5(flow5), cost(cost), cap(cap), flow(flow), isForward(isForward) {}
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
    vector<int> siteGatherId2GraphId;   // [s] ��Ե�ڵ�s�Ļ�۵�
    vector<PII> graphId2ServeId;        // ͼ�Ͻڵ�i����Ӧ����ʵ�ı�Ե�ڵ��±�[t][k]
    vector<int> graphIdType;            // ͼ�Ͻڵ�����ͣ�0Դ�㣬1��㣬2�û���3��Ե�ڵ㣬4��Ե�ڵ�ʱ���۵�

    /************************************ ͳ������ **************************************/
    vector<vector<int64_t>> sitesFlow;               // [t][s]��ʾtʱ��s�ڵ��ϵ�������
    vector<vector<vector<int64_t>>> flowOfUser2Site; // ͳ���û�����Ե�ڵ������

public:
    Solution(SiteList &siteList, Demand &demand, Qos &qos, int qosConstraint)
        : siteList(siteList), demand(demand), qos(qos), qosConstraint(qosConstraint)
    {
        /************************* ���ݼ���Ϣ���� **********************************/
        // ��ȡ���ݼ�����
        numMoments = demand.getNumMoments();
        numSites = siteList.getNumSites();
        numUsers = demand.getNumUsers();
        num5moments = numMoments / 20;
        num95moments = numMoments - num5moments;
        // Ԥ����ÿ����Ե�ڵ�����ӵ��û�����/ÿ���û������ӵı�Ե�ڵ㼯��
        siteConnUserIds.resize(numSites);
        userConnSiteIds.resize(numUsers);
        for (int s = 0; s < numSites; ++s)
        {
            for (int u = 0; u < numUsers; ++u)
                if (qos.get(s, u) < qosConstraint)
                {
                    siteConnUserIds[s].push_back(u);
                    userConnSiteIds[u].push_back(s);
                }
        }
        // ���е��б����������ӶԷ�������
        for (int s = 0; s < numSites; ++s) // ÿ����Ե�ڵ������ӵ��û��������û������ӱ�Ե�ڵ�������
            sort(siteConnUserIds[s].begin(), siteConnUserIds[s].end(), [=](int i, int j)
                 { return userConnSiteIds[i].size() < userConnSiteIds[j].size(); });
        for (int u = 0; u < numUsers; ++u)
            sort(userConnSiteIds[u].begin(), userConnSiteIds[u].end(), [=](int i, int j)
                 { return siteConnUserIds[i].size() < siteConnUserIds[j].size(); });
        for (int s = 0; s < numSites; ++s)
            if (siteConnUserIds[s].size() > 0)
                sortedSiteIds.push_back(s);
        for (int u = 0; u < numUsers; ++u)
            if (userConnSiteIds[u].size() > 0)
                sortedUserIds.push_back(u);
        sort(sortedSiteIds.begin(), sortedSiteIds.end(), [=](int i, int j)
             { return siteConnUserIds[i].size() < siteConnUserIds[j].size(); }); //���������û�����������
        sort(sortedUserIds.begin(), sortedUserIds.end(), [=](int i, int j)
             { return userConnSiteIds[i].size() < userConnSiteIds[j].size(); }); //���տ����ӱ�Ե�ڵ����������

        /******************************* ̰�Ĳ����ڴ� *****************************/
        // ��ʼ��5%�������
        is5.resize(numMoments);
        for (auto &sites : is5)
            sites.resize(numSites, false);

        /****************************** ��ʼ��ͼ�ڴ� *****************************/
        // 2��Դ���� + �û��� + ��Ե�ڵ� + ��Ե�ڵ�ʱ����ܽڵ�
        graph.resize(2 + numUsers * numMoments + numSites * numMoments + numSites);
        userId2GraphId.resize(numMoments);
        siteId2GraphId.resize(numMoments);
        for (int t = 0; t < numMoments; ++t)
        {
            userId2GraphId[t].resize(numUsers);
            siteId2GraphId[t].resize(numSites);
        }
        siteGatherId2GraphId.resize(numSites);
        graphId2ServeId.resize(graph.size()); /// ͼ�ϵ���->��Ե�ڵ���
        graphIdType.resize(graph.size());
        // ������
        superSource = Edge::assginId();
        superDest = Edge::assginId();
        graphIdType[superSource] = 0;
        graphIdType[superDest] = 1;
        for (int t = 0; t < numMoments; ++t)
            for (int u = 0; u < numUsers; ++u)
            {
                userId2GraphId[t][u] = Edge::assginId();
                graphIdType[userId2GraphId[t][u]] = 2;
            }
        for (int t = 0; t < numMoments; ++t)
            for (int s = 0; s < numSites; ++s)
            {
                siteId2GraphId[t][s] = Edge::assginId();
                graphId2ServeId[siteId2GraphId[t][s]] = PII(t, s);
                graphIdType[siteId2GraphId[t][s]] = 3;
            }
        for (int s = 0; s < numSites; ++s)
        {
            siteGatherId2GraphId[s] = Edge::assginId(); // ��Ե�ڵ�s��Ӧ��ʱ���۵�
            graphIdType[siteGatherId2GraphId[s]] = 4;
        }
        /*************************** ͳ������ ***************************/
        sitesFlow.resize(numMoments);
        for (auto &sites : sitesFlow)
            sites.resize(numSites, 0);
        flowOfUser2Site.resize(numMoments);
        for (auto &i : flowOfUser2Site)
        {
            i.resize(numSites);
            for (auto &j : i)
                j.resize(numUsers, 0);
        }
    }

    // ���ڽӱ�����ӱ�
    void addEdge(int from, int to, PLL cap5, int64_t cost, int64_t cap) //���from->to�����
    {
        graph[from].push_back(Edge(from, to, cap5, PLL(0, 0), cost, cap, 0, true));
        graph[to].push_back(Edge(to, from, PLL(0, 0), PLL(0, 0), -cost, 0, 0, false)); //���ڱ�:����0,����0
        graph[from].back().reversedEdge = PII(to, graph[to].size() - 1);
        graph[to].back().reversedEdge = PII(from, graph[from].size() - 1);
    }

    // ��ȡ�����
    Edge &getReversedEdge(Edge &edge)
    {
        return graph[edge.reversedEdge.first][edge.reversedEdge.second];
    }

    // ��ͼ
    void buildGraph(int64_t defaultCost = 1)
    {
        for (int t = 0; t < numMoments; ++t)
        {
            // ����Դ��->user�����������<����inf, ����demand>, ����1������=demand
            for (int &u : sortedUserIds)
                addEdge(superSource, userId2GraphId[t][u], PLL(INF, demand.getDemand(t, u)), defaultCost, demand.getDemand(t, u));
            // user->site�����������<����inf, ����INF>������1������=INF
            for (int s = 0; s < numSites; ++s)
            {
                for (int &u : siteConnUserIds[s])
                    addEdge(userId2GraphId[t][u], siteId2GraphId[t][s], PLL(INF, INF), defaultCost, INF);
            }
            // �����￪ʼ��ͼ1�����������ͼ2���ڷ�����
            // ͼ1��site->siteʱ���۵㣬���������<����1, ����>������0������=0
            for (int &s : sortedSiteIds)
            {
                addEdge(siteId2GraphId[t][s], siteGatherId2GraphId[s], PLL(1, siteList.getBandwidth(s)), 0, 0);
                // ͼ1��siteʱ���۵�->������㣬���������<����5%, ����INF>�� ����0������0
                if (t == 0) // ֻ��һ�α߼���
                    addEdge(siteGatherId2GraphId[s], superDest, PLL(num5moments, INF), 0, 0);
            }
            // ͼ2��site->��㣬��ߣ����������<0, 0>��Ĭ�ϴ���1������֮��=����
            for (int &s : sortedSiteIds)
            {
                int64_t bandwidth = siteList.getBandwidth(s);
                if (!is5[t][s]) // todo
                {
                    vector<int64_t> flows({bandwidth / 6,
                                           bandwidth / 3,
                                           bandwidth / 2,
                                           bandwidth});
                    for (uint32_t i = 1; i < flows.size(); ++i) // ������ֵ���
                        flows[i] -= flows[i - 1];
                    for (uint32_t i = 0; i < flows.size(); ++i)
                        addEdge(siteId2GraphId[t][s], superDest, PLL(0, 0), defaultCost * (i + 1), flows[i]);
                }
                else
                {
                    addEdge(siteId2GraphId[t][s], superDest, PLL(0, 0), 0, bandwidth);
                    addEdge(siteId2GraphId[t][s], superDest, PLL(0, 0), 0, 0);
                    addEdge(siteId2GraphId[t][s], superDest, PLL(0, 0), 0, 0);
                    addEdge(siteId2GraphId[t][s], superDest, PLL(0, 0), 0, 0);
                }
            }
        }
        cout << "Meked Graph with " << graph.size() << " nodes." << endl;
    }

    /******************************* ʹ���������ѡ5% ******************************/

    // spfa�㷨������
    bool spfaForFinding5Sites(int source, int dest, vector<int64_t> &level)
    {
        static vector<int64_t> locks(graph.size());
        locks[source] = INF;
        static vector<bool> inQ(graph.size());
        for (uint32_t i = 0; i < graph.size(); ++i)
        {
            level[i] = INF;
            inQ[i] = false;
        }
        level[source] = 0;
        inQ[source] = true;
        deque<int> Q;
        Q.push_back(source);
        while (!Q.empty())
        {
            int u = Q.front();
            Q.pop_front();
            inQ[u] = false;
            for (auto &e : graph[u])
            {
                // ����siteʱ���۵㣬�������������ֻ������������Է���(x>0)��
                // <0,x> => ����
                // <1,x> && ��һ���������� => ����
                // ������ͨ�㣬ֻ��һ��������У�
                // <����,x>
                // ���ϣ���������еģ�<h,x>, 1.x==0�϶������У�2.h>0 && x>0 && ��һ����û������
                bool allow = false;
                if (PLLMinus(e.cap5, e.flow5).second > 0)
                {
                    if (graphIdType[u] == 4) // ���⣬siteʱ���۵�
                    {
                        if (locks[u] == 0 || (locks[u] > 0 && PLLMinus(e.cap5, e.flow5).first > 0)) // ��>0��Կ��>0������
                            allow = true;
                    }
                    else
                        allow = true;
                }
                if (allow && level[e.to] > level[u] + 1)
                {
                    // cout << PLLMinus(e.cap5, e.flow5).first << "  --  " << PLLMinus(e.cap5, e.flow5).second << " ? " << (PLLMinus(e.cap5, e.flow5) > PLL(0, 0)) << endl;
                    // cout << "node " << e.to << " level:=" << level[u] + 1 << endl;
                    level[e.to] = level[u] + 1;
                    locks[e.to] = min(locks[u], PLLMinus(e.cap5, e.flow5).first); // ���Я������
                    if (!inQ[e.to] && e.to != dest)                               // �����յ�û��Ҫ������
                    {
                        inQ[e.to] = true;
                        if (!Q.empty() && level[e.to] < level[Q.front()])
                            Q.push_front(e.to);
                        else
                            Q.push_back(e.to);
                    }
                }
            }
        }
        return level[dest] < INF;
    }

    // �������������
    PLL dfsForFinding5Sites(int curNode, int dest, vector<int64_t> &level, PLL curFlowLimit)
    {
        if (curNode == dest)
            return curFlowLimit;
        PLL sonsFlowLimit = PLL(0, 0);
        for (auto &edge : graph[curNode])
        {
            if (curFlowLimit == sonsFlowLimit)
                break;
            // ���й���ͬspfaForFinding5Sites
            bool allow = false;
            if (PLLMinus(edge.cap5, edge.flow5).second > 0)
            {
                if (graphIdType[curNode] == 4) // ���⣬siteʱ���۵�
                {
                    if (curFlowLimit.first == 0 || (curFlowLimit.first > 0 && PLLMinus(edge.cap5, edge.flow5).first > 0)) // ��>0��Կ��>0������
                        allow = true;
                }
                else
                    allow = true;
            }
            if (allow && level[curNode] + 1 == level[edge.to])
            {
                // cout << "node " << curNode << " to " << edge.to << "; next level=" << level[edge.to] << "; ";
                // cout << PLLMin(PLLMinus(curFlowLimit, sonsFlowLimit), PLLMinus(edge.cap5, edge.flow5)).first;
                // cout << " " << PLLMin(PLLMinus(curFlowLimit, sonsFlowLimit), PLLMinus(edge.cap5, edge.flow5)).second;
                // cout << " | " << PLLMinus(curFlowLimit, sonsFlowLimit).first << " " << PLLMinus(curFlowLimit, sonsFlowLimit).second;
                // cout << " | " << PLLMinus(edge.cap5, edge.flow5).first << " " << PLLMinus(edge.cap5, edge.flow5).second << endl;
                PLL sonFlow = dfsForFinding5Sites(edge.to, dest, level, PLLMin(PLLMinus(curFlowLimit, sonsFlowLimit), PLLMinus(edge.cap5, edge.flow5)));

                sonsFlowLimit = PLLAdd(sonsFlowLimit, sonFlow);

                edge.flow5 = PLLAdd(edge.flow5, sonFlow);
                getReversedEdge(edge).flow5 = PLLMinus(getReversedEdge(edge).flow5, sonFlow);
            }
        }
        // cout << curNode << " dfs flow " << sonsFlowLimit.first << "," << sonsFlowLimit.second << endl;
        if (sonsFlowLimit == PLL(0, 0)) // ��Ҫ�Ż����˵��Ѿ�û����������ֹ�����ٴη���
            level[curNode] = 0;
        return sonsFlowLimit;
    }

    // �������5%
    void find5sites(int source, int dest)
    {
        static vector<int64_t> level(graph.size()); // ��ǰ������·�㼶
        PLL totalFlow = PLL(0, 0);                  // ������
        while (spfaForFinding5Sites(source, dest, level))
        {
            PLL getFlow = dfsForFinding5Sites(source, dest, level, PLL(INF, INF));
            totalFlow = PLLAdd(totalFlow, getFlow);
            cout << " -- Get flow " << getFlow.first << " " << getFlow.second << endl;
            // system("PAUSE");
        }
        if (DEBUG)
            cout << " -- Total 5% site selected: " << totalFlow.first << "; total 5% flow:" << totalFlow.second << endl;

        /****************************** ͳ��ͼ���ҵ���5%������ʣ�������ƽ�� ******************************/
        // �������ͳ�Ʊ�Ȼ��ͳ��ͼ������
        for (int t = 0; t < numMoments; ++t)
            for (int s = 0; s < numSites; ++s)
            {
                sitesFlow[t][s] = 0;
                for (int u = 0; u < numUsers; ++u)
                    flowOfUser2Site[t][s][u] = 0;
            }
        // ͳ��ͼ�ϵ��������
        vector<vector<int>> demandInfo(numMoments, vector<int>(numUsers));
        for (int t = 0; t < numMoments; ++t)
            for (int u = 0; u < numUsers; ++u)
                demandInfo[t][u] = demand.getDemand(t, u);
        for (int t = 0; t < numMoments; ++t)
        {
            for (int u = 0; u < numUsers; ++u)
            {
                for (auto &e : graph[userId2GraphId[t][u]])
                {
                    if (e.isForward && e.flow5.second < 0)
                    {
                        cout << "ERROR: ����������������Ϊ����; " << endl;
                        cout << e.flow5.first << ":" << e.flow5.second << endl;
                        exit(7);
                    }
                    if (e.isForward && graphIdType[e.to] == 4 && e.flow5.first == 0 && e.flow5.second > 0)
                    {
                        cout << "ERROR: �������ʱ���۱�û�����ͷ�����; " << endl;
                        cout << e.flow5.first << ":" << e.flow5.second << endl;
                        exit(7);
                    }
                    if (e.isForward && e.flow5.second > 0)
                    {
                        int moment = graphId2ServeId[e.to].first;
                        int siteId = graphId2ServeId[e.to].second;
                        flowOfUser2Site[moment][siteId][u] += e.flow5.second;
                        sitesFlow[moment][siteId] += e.flow5.second;
                        demandInfo[t][u] -= e.flow5.second;
                    }
                }
            }
        }
        // �ѷ�����5%�����ܲ���5%����demandInfoΪ��������; ��������ʣ���95%�����������󣬾Ͱ�ƽ��ֵ�ַ�����Ե�ڵ�
        for (int t = 0; t < numMoments; ++t)
        {
            for (int &uid : sortedUserIds) // ÿ���û�ƽ���ַ�
            {
                int64_t avgFlow = upperDiv(demandInfo[t][uid], userConnSiteIds[uid].size()); // ƽ����ÿ����Ե�ڵ��Ϸ��͵�����
                while (demandInfo[t][uid] > 0)                                               // �п���һ��֮������û��������
                {
                    for (int &sid : userConnSiteIds[uid])
                    {
                        int flow = min(avgFlow, min((int64_t)demandInfo[t][uid], siteList.getBandwidth(sid) - sitesFlow[t][sid])); // ������
                        sitesFlow[t][sid] += flow;
                        flowOfUser2Site[t][sid][uid] += flow;
                        demandInfo[t][uid] -= flow;
                    }
                }
            }
        }
    }

    /******************************* ʹ�÷���������ȫ���Ż� ******************************/
    // ����ָ����Ե�ڵ㵽�������Ĵ��ۣ�ע�ⷴ�ڱߴ���ȡ��
    void reassignCostOfSite(int epoch)
    {
        // ����ĳһ���ߵĴ��ۺ�����
        static auto resetCostOfEdge = [&](Edge &edge, int64_t cost, int64_t cap)
        {
            edge.cost = cost;
            getReversedEdge(edge).cost = -cost;
            edge.cap = cap;
        };
        // ����ÿ���ڵ㣬��95�Ʒ�ԭ���ҳ�5%�ڵ�
        for (int siteId = 0; siteId < numSites; ++siteId)
        {
            // ͳ�������������
            static vector<pair<int64_t, int>> sortedFlows(numMoments);
            for (int t = 0; t < numMoments; ++t)
            {
                sortedFlows[t].first = 0;
                sortedFlows[t].second = t; // ����ʵ��ʱ��
                for (auto &edge : graph[siteId2GraphId[t][siteId]])
                {
                    if (edge.isForward && edge.flow > 0)
                    {
                        sortedFlows[t].first += edge.flow;
                    }
                }
            }
            sort(sortedFlows.begin(), sortedFlows.end());
            // for (int i = 0; i < numMoments; ++i)
            //     is5[sortedFlows[i].second][siteId] = (i >= num95moments);
            // �趨��ֵ��ָ������ todo
            double composeLower = 1 - pow(0.91, epoch); // (1-adjustRange -> 1.0) Ӱ��޴�
            double composeUpper = 1 + pow(0.9, epoch);  // Ӱ��΢С
            vector<int64_t> threshold({0,
                                       //    (int64_t)(sortedFlows[num95moments - 1].first / 2),
                                       (int64_t)(sortedFlows[num95moments - 1].first * composeLower),
                                       sortedFlows[num95moments - 1].first,
                                       //    sortedFlows[num95moments].first,
                                       min((int64_t)(sortedFlows[num95moments - 1].first * composeUpper), (int64_t)siteList.getBandwidth(siteId)),
                                       siteList.getBandwidth(siteId)}); // ��ֵ
            // �޸ıߴ���
            for (auto &flowOfmoment : sortedFlows)
            {
                int level = 0;
                int moment = flowOfmoment.second;
                for (Edge &edge : graph[siteId2GraphId[moment][siteId]])
                    if (edge.isForward)
                    {
                        ++level;
                        // if (!is5[moment][siteId]) // С��96; ��4���� todo
                        if (flowOfmoment.first <= sortedFlows[num95moments - 1].first) // С��96; ��4����
                            resetCostOfEdge(edge, level, threshold[level] - threshold[level - 1]);
                        else // ���5%ֱ�����أ��������
                        {
                            if (level == 1)
                                resetCostOfEdge(edge, 1, siteList.getBandwidth(siteId));
                            else
                                resetCostOfEdge(edge, 0, 0);
                        }
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
        deque<int> Q;
        Q.push_back(source);
        while (!Q.empty())
        {
            int u = Q.front();
            Q.pop_front();
            inQ[u] = false;
            for (auto &e : graph[u])
            {
                if (e.cap - e.flow > 0 && cost[e.to] > cost[u] + e.cost) // ֻҪ������ && ·�����
                {
                    level[e.to] = level[e.from] + 1;
                    cost[e.to] = cost[u] + e.cost;
                    preEdge[e.to] = &e;             // ��סǰ���ߵĵ�ַ
                    if (!inQ[e.to] && e.to != dest) // �����յ�û��Ҫ������
                    {
                        inQ[e.to] = true;
                        if (!Q.empty() && cost[e.to] < cost[Q.front()])
                            Q.push_front(e.to);
                        else
                            Q.push_back(e.to);
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
        if (sonsFlowLimit == 0) // ��Ҫ�Ż����˵��Ѿ�û����������ֹ�����ٴη���
            level[curNode] = 0;
        return sonsFlowLimit;
    }

    // zkw�㷨�����С������
    void mincost(int source, int dest)
    {
        static vector<int64_t> cost(graph.size()); // ��ǰ��Ļ���
        // static vector<int64_t> flow(graph.size());   // ��ǰ�������
        static vector<int> level(graph.size());      // ��ǰ������·�㼶
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
        if (DEBUG)
            cout << " -- Total flow " << totalFlow << "; Total cost " << totalCost << endl;
    }

    /******************************** ͳ����Ϣ ***********************************/
    // ͳ�Ʒ�����ͼ������
    void gatherGraphFlow()
    {
        // �������ͳ�Ʊ�Ȼ��ͳ��ͼ������
        for (int t = 0; t < numMoments; ++t)
            for (int s = 0; s < numSites; ++s)
            {
                sitesFlow[t][s] = 0;
                for (int u = 0; u < numUsers; ++u)
                    flowOfUser2Site[t][s][u] = 0;
            }
        // ͳ��ͼ�ϵ��������
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
                        flowOfUser2Site[moment][siteId][u] += e.flow; // �������
                        sitesFlow[moment][siteId] += e.flow;          // ���ڼ���ɱ�
                    }
                }
            }
        }
    }

    // �����ܳɱ� 95�Ʒ�
    int64_t calclateProblemCost()
    {
        int64_t cost = 0;
        for (int s = 0; s < numSites; ++s)
        {
            static vector<int64_t> flows(numMoments);
            for (int t = 0; t < numMoments; ++t)
                flows[t] = sitesFlow[t][s];
            sort(flows.begin(), flows.end());
            cost += flows[num95moments - 1];
        }
        return cost;
    }

    // ����������ļ�
    void output(string filepath)
    {
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
                    if (flowOfUser2Site[t][s][u] > 0)
                    {
                        if (first)
                            first = false;
                        else
                            fout << ',';
                        fout << '<' << siteList.getSiteName(s) << ',' << flowOfUser2Site[t][s][u] << '>';
                    }
                }
                fout << endl;
            }
        }
        fout.close();
        cout << "Solution has been written to file " << filepath << endl;
    }

    /******************************** ���������߼� ********************************/
    void run(int epochs = 50)
    {
        auto startClock = clock();
        buildGraph();
        /************************** �������ѡ5% *************************/
        find5sites(superSource, superDest);
        return;
        /************************* ������ȫ���Ż� ************************/
        mincost(superSource, superDest); // ��ʼ��һ����
        for (int epoch = 0; epoch < epochs; epoch++)
        {
            auto epochClock = clock();
            if (DEBUG)
                cout << "Epoch " << epoch << ": " << endl;
            reassignCostOfSite(epoch); // �Ƚ��������㣬�����ô���
            mincost(superSource, superDest);
            if (DEBUG)
            {
                gatherGraphFlow();
                cout << " -- Epoch Time Spent " << (clock() - epochClock) << "ms. Problem Cost " << calclateProblemCost() << endl;
            }
            if ((clock() - startClock) / 1000000 > 290)
                break;
        }
        gatherGraphFlow(); // ������������ͳ��ͼ������
        /***************************** ���Խ��� *****************************/
        if (DEBUG)
            cout << "Total time spent " << (clock() - startClock) << endl;
    }
};

int main()
{
    /******** ȷ�����ݼ�·�� ********/
    string dataDir = "/data", outputPath = "/output/solution.txt"; // Ĭ������·��
    string linuxDir = "../dataset/data";                           // linux����·��
    // string linuxDir = "../dataset/data200"; // linux����·��
    // string linuxDir = "../dataset/pressure0.6_data"; // linux����·��
    // string linuxDir = "../dataset/pressure0.3-t8926-n135-m35"; // linux����·��
    // string linuxDir = "../dataset/pressure0.3-t200-n100-m20"; // linux����·��
    string winDir = "../" + linuxDir;             // win����·��
    if (ifstream(dataDir + "/config.ini").good()) // ���ϻ���
    {
        DEBUG = false;
    }
    else if (ifstream(winDir + "/config.ini").good()) // windows����
    {
        dataDir = winDir;
        outputPath = winDir + "/solution.txt";
    }
    else if (ifstream(linuxDir + "/config.ini").good()) // linux���Ի���
    {
        dataDir = linuxDir;
        outputPath = linuxDir + "/solution.txt";
    }
    if (DEBUG)
        cout << "------------------------- DEBUG mode --------------------------" << endl;

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
