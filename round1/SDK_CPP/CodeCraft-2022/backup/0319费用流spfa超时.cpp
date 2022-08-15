#include <bits/stdc++.h>
using namespace std;
typedef pair<int, int> PII;
typedef long long LL;
const LL INF = (1LL << 50);

// 去除字符串首尾空白符
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

// 读取配置文件config.ini
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
            if (eq_pos != -1 && line.substr(0, eq_pos) == keyName) //找到配置项
                return line.substr(eq_pos + 1);
        }
    }
    return retValue;
}

// 边缘节点表
class SiteList
{
private:
    vector<string> siteNames;   //边缘节点名字列表
    map<string, int> siteId;    //边缘节点名：编号
    vector<int> siteBandwidths; //边缘节点带宽上限表

public:
    explicit SiteList(string filepath) //读取文件构造方法
    {
        ifstream fin(filepath, ios::in);
        if (!fin)
        {
            cout << "Can't open file " << filepath << endl;
            exit(3);
        }
        string line, siteName, bandwidth;
        getline(fin, line); // 读取表头
        while (getline(fin, line))
        {
            istringstream sin(strip(line));
            getline(sin, siteName, ',');  // 读取节点名
            getline(sin, bandwidth, ','); // 读取带宽限制
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

// 用户需求表
class Demand
{
private:
    vector<string> moments;     // 时刻表
    vector<string> userNames;   // 用户名表
    map<string, int> userId;    // 用户名：用户编号
    vector<vector<int>> demand; // 需求表

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
        // 读取表头
        getline(fin, line);
        istringstream sin(strip(line));
        getline(sin, username, ','); // mtime
        while (getline(sin, username, ','))
            userNames.push_back(strip(username));
        for (int i = 0; i < (int)userNames.size(); ++i)
            userId[userNames[i]] = i;
        // 读取需求值
        for (int t = 0; getline(fin, line); t++) // t代表时刻
        {
            istringstream sin(strip(line));
            getline(sin, moment, ',');
            moments.push_back(moment);
            vector<int> dem; //当前时刻的用户需求量
            while (getline(sin, bandwidth, ','))
            {
                dem.push_back(atoi(bandwidth.c_str()));
            }
            demand.push_back(dem); // t时刻的所有需求
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

// 时延表
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
        // 初始化qos大小
        qos.resize(siteList.getNumSites());
        for (auto &e : qos)
            e.resize(demand.getNumUsers());

        /******** 读取数据 ***********/
        string line, siteName, username, q;
        // 读取第一行用户名，顺便映射为编号
        getline(fin, line);
        istringstream sin(strip(line));
        getline(sin, username, ',');
        vector<int> userIds(demand.getNumUsers());
        for (int u = 0; getline(sin, username, ','); ++u)
        {
            userIds[u] = demand.getUserId(username); //获取用户编号
        }
        // 读取时延矩阵qos
        for (int t = 0; getline(fin, line); t++) // t代表时刻
        {
            istringstream sin(strip(line));
            getline(sin, siteName, ',');
            int siteId = siteList.getSiteId(siteName); //获取边缘节点编号
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

// 解决方案
class Solution
{
public:
    // 数据集
    SiteList &siteList; //边缘节点集
    Demand &demand;     //需求集
    Qos &qos;           //时延表
    int qosConstraint;  //时延上限

    // 数据集属性
    int numMoments;   //时刻总数
    int numSites;     //边缘节点总数
    int numUsers;     //用户总数
    int num95moments; // 95百分位时刻数量
    int num5moments;  // 5百分位时刻数量

    // 记录实时流量
    vector<vector<vector<int>>> userFlow; // userFlow[t][s][u]表示t时刻节点s与用户u之间的流量
    vector<vector<int>> siteFlow;         // siteFlow[t][s]表示t时刻s节点上的总流量

    // 图
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
    int superSource, superDest;         // 超级源点, 超级汇点
    vector<vector<EKEdge>> graph;       // 邻接表
    vector<vector<int>> siteId2GraphId; // [t][s] t时刻边缘节点s的图编号
    vector<vector<int>> userId2GraphId; // [t][u] t时刻用户节点u的图编号
    vector<PII> graphId2ServeId;         // 图上节点i所对应的真实的边缘节点下标[t][u]

    Solution(SiteList &siteList, Demand &demand, Qos &qos, int qosConstraint)
        : siteList(siteList), demand(demand), qos(qos), qosConstraint(qosConstraint)
    {
        // 获取数据集属性
        numMoments = demand.getNumMoments();
        numSites = siteList.getNumSites();
        numUsers = demand.getNumUsers();
        num5moments = numMoments / 20;
        num95moments = numMoments - num5moments;
        // 初始化userFlow
        userFlow.resize(numMoments);
        for (auto &i : userFlow)
        {
            i.resize(numSites);
            for (auto &j : i)
            {
                j.resize(numUsers);
                for (auto &e : j)
                    e = 0; // 初始化每个客户流量为0
            }
        }
        // 初始化siteFlow
        siteFlow.resize(numMoments);
        for (auto &i : siteFlow)
        {
            i.resize(numSites);
            for (auto &j : i)
                j = 0; // 初始化每个节点总流量为0
        }
        // 初始化图
        superSource = 0;
        superDest = 1;
        graph.resize(numMoments * (numUsers + numSites) + 2); // 0源点 1汇点
        siteId2GraphId.resize(numMoments);
        userId2GraphId.resize(numMoments);
        for (int t = 0; t < numMoments; ++t)
        {
            siteId2GraphId[t].resize(numSites);
            userId2GraphId[t].resize(numUsers);
        }
        graphId2ServeId.resize(graph.size());
    }

    // 向邻接表中添加边
    void addEKEdge(int from, int to, LL cost, LL cap) //添加from->to有向边
    {
        graph[from].push_back(EKEdge(from, to, cost, cap, 0, true));
        graph[to].push_back(EKEdge(to, from, -cost, 0, 0, false)); //反悔边
        graph[from].back().reversedEdge = PII(to, graph[to].size() - 1);
        graph[to].back().reversedEdge = PII(from, graph[from].size() - 1);
    }

    // 建图
    void makeEKGraph()
    {
        // 分配编号
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
        // 建图
        for (int t = 0; t < numMoments; ++t)
        {
            // user->site连边,代价0，容量无限
            for (int s = 0; s < numSites; ++s)
            {
                for (int u = 0; u < numUsers; ++u)
                {
                    if (qos.get(s, u) >= qosConstraint)
                        continue;
                    addEKEdge(userId2GraphId[t][u], siteId2GraphId[t][s], 0, INF);
                }
            }
            // site-超级汇点，代价1，容量=带宽
            for (int s = 0; s < numSites; ++s)
                addEKEdge(siteId2GraphId[t][s], superDest, 1, siteList.getBandwidth(s));
            // 超级源点-user，代价0，容量=需求
            for (int u = 0; u < numUsers; ++u)
                addEKEdge(superSource, userId2GraphId[t][u], 0, demand.getDemand(t, u));
        }
        cout << "Meked Graph with number of node: " << graph.size() << endl;
    }

    // spfa算法求增广路
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
                if (e.cap - e.flow > 0 && cost[e.to] > cost[u] + e.cost) // 只要有流量，并且路径变短
                {
                    flow[e.to] = min(flow[u], e.cap - e.flow);
                    cost[e.to] = cost[u] + e.cost;
                    preEdge[e.to] = &e; //记住前驱边的地址
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
        static vector<LL> cost(graph.size()), flow(graph.size()); // 当前点的花费和流量
        static vector<EKEdge *> preEdge(graph.size());            // 当前点的前驱边（指针）
        LL totalFlow = 0, totalCost = 0;                   // 总流量，总花费
        while (spfa(source, dest, cost, flow, preEdge))
        {
            // cout << " -- Found a augmenting path with flow " << flow[dest] << " and cost " << cost[dest] << endl;
            totalFlow += flow[dest];              // 本条增广路的流量
            totalCost += cost[dest] * flow[dest]; // 代价
            for (int p = dest; p != source; p = preEdge[p]->from)
            {
                preEdge[p]->flow += flow[dest];
                graph[preEdge[p]->reversedEdge.first][preEdge[p]->reversedEdge.second].flow -= flow[dest];
            }
        }
        // 统计流量
        cout << "Start to gather flow with total flow " << totalFlow << " and total cost " << totalCost << endl;
        for (int t = 0; t < numMoments; ++t)
        {
            for (int u = 0; u < numUsers; ++u)
            {
                for (auto &e : graph[userId2GraphId[t][u]]) // user起点, 所指向的site必然是同时刻的
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

    // 将答案输出到文件
    void output(string filepath)
    {
        cout << "Start to output solution." << endl;
        ofstream fout(filepath, ios::out);

        for (int t = 0; t < numMoments; ++t)
        {
            //对于每一个时刻，遍历用户
            for (int u = 0; u < numUsers; ++u)
            {
                //对于t时刻的用户u，输出其连接的所有流量大于0的边缘节点
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
    /******** 确定数据集路径 ********/
    string dataDir = "/data", outputPath = "/output/solution.txt"; // 默认线上环境
    if (ifstream("../data/config.ini").good())                     // windows测试环境
    {
        // dataDir = "../data";
        // outputPath = "../data/solution.txt";
        dataDir = "../pressure0.6_data";
        outputPath = "../pressure0.6_data/solution.txt";
    }
    else if (ifstream("../CodeCraft-2022/data/config.ini").good()) // linux测试环境
    {
        dataDir = "../CodeCraft-2022/data";
        outputPath = "../CodeCraft-2022/data/solution.txt";
    }

    /********* 读取数据集 *********/
    SiteList siteList(dataDir + "/site_bandwidth.csv");
    Demand demand(dataDir + "/demand.csv");
    Qos qos(dataDir + "/qos.csv", siteList, demand);
    int qosConstraint = atoi(getConfigIni(dataDir + "/config.ini", "config", "qos_constraint").c_str());

    /********* 执行解决方案 *********/
    Solution solution(siteList, demand, qos, qosConstraint);
    int cost = solution.run();
    cout << "Final Cost: " << cost << endl;

    /********* 将结果写入文件 *********/
    solution.output(outputPath);
    return 0;
}
