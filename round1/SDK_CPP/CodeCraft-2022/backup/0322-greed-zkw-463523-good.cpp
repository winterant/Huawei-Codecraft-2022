#include <bits/stdc++.h>
using namespace std;
typedef pair<int, int> PII;
const int64_t INF = (1LL << 40);
bool DEBUG = true; // 标记是否线下

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

    vector<vector<int>> cloneDemand()
    {
        return demand;
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
private:
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

    // 每个边缘节点可连接的用户集合
    vector<vector<int>> siteConnUsers;

    // 统计流量
    vector<vector<int64_t>> netSiteFlow;             // [t][s]表示t时刻s节点上的总流量
    vector<vector<vector<int64_t>>> flowOfUser2Site; // 统计用户到边缘节点的流量

    // 标记当前是不是5%节点
    vector<vector<bool>> is5;

    // 图
    struct Edge
    {
        int from, to;
        int64_t cost;
        int64_t cap, flow;
        bool isForward;
        PII reversedEdge; // <node id, edge index>
        Edge(int from, int to, int64_t cost, int64_t cap, int64_t flow, bool isForward) : from(from), to(to), cost(cost), cap(cap), flow(flow), isForward(isForward) {}
        // 为图上节点编号分配一个编号
        static int assginId()
        {
            static int nodeId = 0;
            return nodeId++;
        }
    };
    // 使用邻接表保存图，并保存原题节点与图上节点的对应关系
    int superSource, superDest;         // 超级源点，超级汇点
    vector<vector<Edge>> graph;         // 邻接表
    vector<vector<int>> userId2GraphId; // [t][u] 用户节点u的图编号
    vector<vector<int>> siteId2GraphId; // [t][s] t时刻边缘节点s的图编号
    vector<PII> graphId2ServeId;        // 图上节点i所对应的真实的边缘节点下标[t][k]

public:
    Solution(SiteList &siteList, Demand &demand, Qos &qos, int qosConstraint)
        : siteList(siteList), demand(demand), qos(qos), qosConstraint(qosConstraint)
    {
        // 获取数据集属性
        numMoments = demand.getNumMoments();
        numSites = siteList.getNumSites();
        numUsers = demand.getNumUsers();
        num5moments = numMoments / 20;
        num95moments = numMoments - num5moments;
        // 预处理每个边缘节点可连接的用户集合
        siteConnUsers.resize(numSites);
        for (int s = 0; s < numSites; ++s)
        {
            for (int u = 0; u < numUsers; ++u)
                if (qos.get(s, u) < qosConstraint)
                    siteConnUsers[s].push_back(u);
        }
        // 初始化netSiteFlow
        netSiteFlow.resize(numMoments);
        for (auto &sites : netSiteFlow)
            sites.resize(numSites, 0);
        flowOfUser2Site.resize(numMoments);
        for (auto &i : flowOfUser2Site)
        {
            i.resize(numSites);
            for (auto &j : i)
                j.resize(numUsers, 0);
        }
        // 初始化5%标记数组
        is5.resize(numMoments);
        for (auto &sites : is5)
            sites.resize(numSites, false);
        // 初始化图
        graph.resize(2 + numMoments * (numUsers + numSites)); // 图上节点的个数：2个源汇点，user/site各复制T倍
        userId2GraphId.resize(numMoments);
        siteId2GraphId.resize(numMoments);
        for (int t = 0; t < numMoments; ++t)
        {
            userId2GraphId[t].resize(numUsers);
            siteId2GraphId[t].resize(numSites);
        }
        graphId2ServeId.resize(graph.size());
    }

    // 首先使用贪心，满载5%；此函数仅标记5%到数组is5，不作实际修改
    void greed()
    {
        // 统计每个边缘节点可连接的用户数量并排序
        static vector<PII> sitesSortedByUsers(numSites);
        for (int s = 0; s < numSites; ++s)
        {
            sitesSortedByUsers[s].first = siteConnUsers[s].size();
            sitesSortedByUsers[s].second = s;
        }
        sort(sitesSortedByUsers.begin(), sitesSortedByUsers.end());
        // 将每个边缘节点选出5%个时间点满载
        auto demandTemp = demand.cloneDemand();
        for (auto &p : sitesSortedByUsers)
        {
            int siteId = p.second;
            // 统计当前边缘节点每个时间片的可满载值
            static vector<pair<int64_t, int>> flows(numMoments);
            for (int t = 0; t < numMoments; ++t)
            {
                int bandwidth = siteList.getBandwidth(siteId); // 边缘节点带宽限制
                flows[t].first = 0;
                flows[t].second = t;
                // 统计一下当前时刻，节点的可满载需求量
                for (int &u : siteConnUsers[siteId])
                {
                    int flow = min(demandTemp[t][u], bandwidth);
                    if (flow == 0)
                        break;
                    flows[t].first += flow;
                    bandwidth -= flow;
                }
            }
            // 按可满载需求量从大到小排序
            sort(flows.begin(), flows.end(), greater<pair<int64_t, int>>());
            // 对最大的5%进行满载
            for (int i = 0; i < num5moments; i++)
            {
                int maxBandwidth = flows[i].first;
                int moment = flows[i].second;
                for (int &u : siteConnUsers[siteId])
                {
                    int flow = min(demandTemp[moment][u], maxBandwidth);
                    if (flow <= 0)
                        break;
                    maxBandwidth -= flow;
                    demandTemp[moment][u] -= flow;
                    is5[moment][siteId] = true; // 标记为5%节点；建图时直接设为代价最低
                }
            }
        }
    }

    // 向邻接表中添加边
    void addEdge(int from, int to, int64_t cost, int64_t cap) //添加from->to有向边
    {
        graph[from].push_back(Edge(from, to, cost, cap, 0, true));
        graph[to].push_back(Edge(to, from, -cost, 0, 0, false)); //反悔边
        graph[from].back().reversedEdge = PII(to, graph[to].size() - 1);
        graph[to].back().reversedEdge = PII(from, graph[from].size() - 1);
    }

    // 获取反向边
    Edge &getReversedEdge(Edge &edge)
    {
        return graph[edge.reversedEdge.first][edge.reversedEdge.second];
    }

    // 建图
    void buildGraph(int64_t defaultCost = 1)
    {
        // 分配编号
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

        // 建图
        for (int t = 0; t < numMoments; ++t)
        {
            // 超级源点->user，代价1，容量=demand
            for (int u = 0; u < numUsers; ++u)
                addEdge(superSource, userId2GraphId[t][u], defaultCost, demand.getDemand(t, u));
            // user->site，代价1，容量=INF
            for (int s = 0; s < numSites; ++s)
            {
                for (int &u : siteConnUsers[s])
                    addEdge(userId2GraphId[t][u], siteId2GraphId[t][s], defaultCost, INF);
            }
            // site->汇点，拆边，默认代价1，容量之和=带宽
            for (int s = 0; s < numSites; ++s)
            {
                int bandwidth = siteList.getBandwidth(s);
                if (!is5[t][s])
                {
                    addEdge(siteId2GraphId[t][s], superDest, defaultCost * 1, bandwidth / 6);
                    addEdge(siteId2GraphId[t][s], superDest, defaultCost * 2, bandwidth / 6);
                    addEdge(siteId2GraphId[t][s], superDest, defaultCost * 3, bandwidth / 6);
                    addEdge(siteId2GraphId[t][s], superDest, defaultCost * 4, bandwidth - bandwidth / 6 * 3);
                }
                else
                {
                    addEdge(siteId2GraphId[t][s], superDest, 0, 0);
                    addEdge(siteId2GraphId[t][s], superDest, 0, 0);
                    addEdge(siteId2GraphId[t][s], superDest, 0, 0);
                    addEdge(siteId2GraphId[t][s], superDest, 0, bandwidth);
                }
            }
        }
        cout << "Meked Graph with number of node: " << graph.size() << endl;
    }

    // 重新指定边缘节点到超级汇点的代价；注意反悔边代价取负
    void reassignCostOfSite(int epoch, int epochs)
    {
        // 重置某一条边的代价和容量
        static auto resetCostOfEdge = [&](Edge &edge, int64_t cost, int64_t cap)
        {
            edge.cost = cost;
            getReversedEdge(edge).cost = -cost;
            edge.cap = cap;
        };
        // 对于每个节点，按95计费原则找出5%节点
        for (int siteId = 0; siteId < numSites; ++siteId)
        {
            // 统计流量分配情况
            static vector<int64_t> flows(numMoments);
            static vector<pair<int64_t, int>> sortedFlows(numMoments);
            for (int t = 0; t < numMoments; ++t)
            {
                flows[t] = sortedFlows[t].first = 0;
                sortedFlows[t].second = t; // 记下实际时间
                for (auto &edge : graph[siteId2GraphId[t][siteId]])
                {
                    if (edge.isForward && edge.flow > 0)
                    {
                        flows[t] += edge.flow;
                        sortedFlows[t].first += edge.flow;
                    }
                }
            }
            sort(sortedFlows.begin(), sortedFlows.end());
            // for (int i = 0; i < numMoments; ++i)
            // is5[sortedFlows[i].second][siteId] = (i < num95moments);
            // 设定阈值，指定代价
            double composeLower = 1 - pow(0.9, epoch); // (1-adjustRange -> 1.0)
            double composeUpper = 1 + pow(0.9, epoch);
            vector<int64_t> threshold({0,
                                       (int64_t)(sortedFlows[num95moments - 1].first * composeLower),
                                       sortedFlows[num95moments - 1].first,
                                       //    sortedFlows[num95moments].first,
                                       min((int64_t)(sortedFlows[num95moments - 1].first * composeUpper), (int64_t)siteList.getBandwidth(siteId)),
                                       siteList.getBandwidth(siteId)}); // 阈值
            // 修改边代价
            for (auto &moments : sortedFlows)
            {
                int level = 0;
                int64_t curFlow = moments.first;
                int moment = moments.second;
                for (Edge &edge : graph[siteId2GraphId[moment][siteId]])
                    if (edge.isForward)
                    {
                        ++level;
                        // if (!is5[t][siteId]) // 小于96; 分4条边
                        if (curFlow <= sortedFlows[num95moments - 1].first) // 小于96; 分4条边
                            resetCostOfEdge(edge, level, threshold[level] - threshold[level - 1]);
                        else // 最后5%直接满载，代价最低
                            resetCostOfEdge(edge, 1, threshold[level] - threshold[level - 1]);
                    }
            }
        }
        // 图流量归零
        for (auto &edges : graph)
            for (auto &e : edges)
                e.flow = 0;
    }

    // spfa算法求流量约束下的最短路
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
                if (e.cap - e.flow > 0 && cost[e.to] > cost[u] + e.cost) // 只要有流量 && 路径变短
                {
                    level[e.to] = level[e.from] + 1;
                    cost[e.to] = cost[u] + e.cost;
                    preEdge[e.to] = &e;             // 记住前驱边的地址
                    if (!inQ[e.to] && e.to != dest) // 到达终点没必要继续走
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

    // 深度搜索可行流
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

    // zkw算法求解最小费用流
    void mincost(int source, int dest)
    {
        static vector<int64_t> cost(graph.size());   // 当前点的花费
        static vector<int64_t> flow(graph.size());   // 当前点的流量
        static vector<int> level(graph.size());      // 当前点的最短路层级
        static vector<Edge *> preEdge(graph.size()); // 当前点的前驱边（指针）
        int64_t totalCost = 0;                       // 总花费
        int64_t totalFlow = 0;                       // 总流量
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
            cout << " -- Total flow " << totalFlow << "; Total cost " << totalCost << "; Problem Cost:" << calclateProblemCost() << endl;
    }

    // 统计网络流图上流量
    void regatherGraphFlow()
    {
        // 清空流量统计表，然后统计图上流量
        for (int t = 0; t < numMoments; ++t)
            for (int s = 0; s < numSites; ++s)
            {
                netSiteFlow[t][s] = 0;
                for (int u = 0; u < numUsers; ++u)
                    flowOfUser2Site[t][s][u] = 0;
            }
        // 统计图上的流量情况
        for (int t = 0; t < numMoments; ++t)
        {
            for (int u = 0; u < numUsers; ++u)
            {
                for (auto &e : graph[userId2GraphId[t][u]])
                {
                    if (e.isForward && e.flow < 0)
                    {
                        cout << "ERROR: 正向边流量为负数" << endl;
                        exit(-2);
                    }
                    if (e.isForward && e.flow > 0)
                    {
                        int moment = graphId2ServeId[e.to].first;
                        int siteId = graphId2ServeId[e.to].second;
                        flowOfUser2Site[moment][siteId][u] += e.flow; // 用于输出
                        netSiteFlow[moment][siteId] += e.flow;        // 用于计算成本
                    }
                }
            }
        }
    }

    // 计算总成本 95计费
    int64_t calclateProblemCost(bool gatherFlow = true)
    {
        if (gatherFlow)
            regatherGraphFlow();
        int64_t cost = 0;
        for (int s = 0; s < numSites; ++s)
        {
            static vector<int64_t> flows(numMoments);
            for (int t = 0; t < numMoments; ++t)
                flows[t] = netSiteFlow[t][s];
            sort(flows.begin(), flows.end());
            cost += flows[num95moments - 1];
        }
        return cost;
    }

    // 将答案输出到文件
    void output(string filepath)
    {
        cout << "Problem Final Cost: " << calclateProblemCost() << endl;
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
        cout << "Solution has been wrote to file " << filepath << endl;
    }

    void run(int epochs = 100)
    {
        auto startClock = clock();
        greed();
        buildGraph();
        mincost(superSource, superDest); // 初始跑一个解
        for (int epoch = 0; epoch < epochs; epoch++)
        {
            auto epochClock = clock();
            if (DEBUG)
                cout << "Epoch " << epoch << ": " << endl;
            reassignCostOfSite(epoch, epochs); // 先将流量清零，并设置代价
            mincost(superSource, superDest);
            if (DEBUG)
                cout << " -- Epoch Time Spent " << (clock() - epochClock) << "ms" << endl;
            if ((clock() - startClock) / 1000000 > 295)
                break;
        }
        if (DEBUG)
            cout << "Total time spent " << (clock() - startClock) << "ms" << endl;
    }
};

int main()
{
    /******** 确定数据集路径 ********/
    string dataDir = "/data", outputPath = "/output/solution.txt"; // 默认线上路径
    // string linuxDir = "../dataset/data";             // linux测试路径
    // string linuxDir = "../dataset/pressure0.6_data"; // linux测试路径
    string linuxDir = "../dataset/pressure0.3-t8926-n135-m35"; // linux测试路径
    string winDir = "../" + linuxDir;                          // win测试路径
    if (ifstream(dataDir + "/config.ini").good())              // 线上环境
    {
        DEBUG = false;
    }
    else if (ifstream(winDir + "/config.ini").good()) // windows环境
    {
        dataDir = winDir;
        outputPath = winDir + "/solution.txt";
    }
    else if (ifstream(linuxDir + "/config.ini").good()) // linux测试环境
    {
        dataDir = linuxDir;
        outputPath = linuxDir + "/solution.txt";
    }
    if (DEBUG)
        cout << "------------------------- DEBUG mode --------------------------" << endl;

    /********* 读取数据集 *********/
    SiteList siteList(dataDir + "/site_bandwidth.csv");
    Demand demand(dataDir + "/demand.csv");
    Qos qos(dataDir + "/qos.csv", siteList, demand);
    int qosConstraint = atoi(getConfigIni(dataDir + "/config.ini", "config", "qos_constraint").c_str());

    /********* 执行解决方案 *********/
    Solution solution(siteList, demand, qos, qosConstraint);
    solution.run();

    /********* 将结果写入文件 *********/
    solution.output(outputPath);
    return 0;
}
