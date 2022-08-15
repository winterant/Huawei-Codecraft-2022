#include <bits/stdc++.h>
using namespace std;
typedef pair<int, int> PII;
typedef pair<int64_t, int> PLI;
typedef pair<int64_t, int64_t> PLL;
const int64_t INF = (1LL << 40);
bool DEBUG = true; // 标记是否线下

// 向上取整的除法
int64_t upperDiv(const int64_t &num, const int64_t &div)
{
    return (num + div - 1) / div;
}

// pair加法
PLL PLLAdd(const PLL &a, const PLL &b)
{
    return PLL(a.first + b.first, a.second + b.second);
}

// pair减法
PLL PLLMinus(const PLL &a, const PLL &b)
{
    return PLL(a.first - b.first, a.second - b.second);
}

PLL PLLMin(const PLL &a, const PLL &b)
{
    return PLL(min(a.first, b.first), min(a.second, b.second));
}
// 去除字符串首尾空白符
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
private:
    /********************** 数据集信息处理 *****************************/
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

    // 编号预处理存储
    vector<vector<int>> siteConnUserIds; // 每个边缘节点可连接的用户集合
    vector<vector<int>> userConnSiteIds; // 每个用户可连接的边缘节点集合
    vector<int> sortedSiteIds;           // 排序的边缘节点编号；无连接的可删除
    vector<int> sortedUserIds;           // 排序的用户编号；无连接的可删除

    /****************************** 贪心策略 *****************************/
    vector<vector<bool>> is5; // 标记当前是不是5%节点

    /******************************** 图 *******************************/
    struct Edge
    {
        // 节点编号
        int from, to;
        // 最大流数据,用于找5%
        PLL cap5;  // <cap1边数限制为5%, cap2带宽限制>
        PLL flow5; // <flow1边数, flow2流量>
        // 费用流数据,用于规划全局
        int64_t cost;
        int64_t cap, flow;
        // 边信息
        bool isForward;
        PII reversedEdge; // <node id, edge index>
        Edge(int from, int to, PLL cap5, PLL flow5, int64_t cost, int64_t cap, int64_t flow, bool isForward)
            : from(from), to(to), cap5(cap5), flow5(flow5), cost(cost), cap(cap), flow(flow), isForward(isForward) {}
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
    vector<int> siteGatherId2GraphId;   // [s] 边缘节点s的汇聚点
    vector<PII> graphId2ServeId;        // 图上节点i所对应的真实的边缘节点下标[t][k]
    vector<int> graphIdType;            // 图上节点的类型：0源点，1汇点，2用户，3边缘节点，4边缘节点时序汇聚点

    /************************************ 统计流量 **************************************/
    vector<vector<int64_t>> sitesFlow;               // [t][s]表示t时刻s节点上的总流量
    vector<vector<vector<int64_t>>> flowOfUser2Site; // 统计用户到边缘节点的流量

public:
    Solution(SiteList &siteList, Demand &demand, Qos &qos, int qosConstraint)
        : siteList(siteList), demand(demand), qos(qos), qosConstraint(qosConstraint)
    {
        /************************* 数据集信息处理 **********************************/
        // 获取数据集属性
        numMoments = demand.getNumMoments();
        numSites = siteList.getNumSites();
        numUsers = demand.getNumUsers();
        num5moments = numMoments / 20;
        num95moments = numMoments - num5moments;
        // 预处理每个边缘节点可连接的用户集合/每个用户可连接的边缘节点集合
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
        // 所有的列表，都按可连接对方数排序
        for (int s = 0; s < numSites; ++s) // 每个边缘节点所连接的用户，按该用户可连接边缘节点数排序
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
             { return siteConnUserIds[i].size() < siteConnUserIds[j].size(); }); //按可连接用户的数量排序
        sort(sortedUserIds.begin(), sortedUserIds.end(), [=](int i, int j)
             { return userConnSiteIds[i].size() < userConnSiteIds[j].size(); }); //按照可连接边缘节点的数量排序

        /******************************* 贪心策略内存 *****************************/
        // 初始化5%标记数组
        is5.resize(numMoments);
        for (auto &sites : is5)
            sites.resize(numSites, false);

        /****************************** 初始化图内存 *****************************/
        // 2个源点汇点 + 用户点 + 边缘节点 + 边缘节点时序汇总节点
        graph.resize(2 + numUsers * numMoments + numSites * numMoments + numSites);
        userId2GraphId.resize(numMoments);
        siteId2GraphId.resize(numMoments);
        for (int t = 0; t < numMoments; ++t)
        {
            userId2GraphId[t].resize(numUsers);
            siteId2GraphId[t].resize(numSites);
        }
        siteGatherId2GraphId.resize(numSites);
        graphId2ServeId.resize(graph.size()); /// 图上点编号->边缘节点编号
        graphIdType.resize(graph.size());
        // 分配编号
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
            siteGatherId2GraphId[s] = Edge::assginId(); // 边缘节点s对应的时序汇聚点
            graphIdType[siteGatherId2GraphId[s]] = 4;
        }
        /*************************** 统计流量 ***************************/
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

    // 向邻接表中添加边
    void addEdge(int from, int to, PLL cap5, int64_t cost, int64_t cap) //添加from->to有向边
    {
        graph[from].push_back(Edge(from, to, cap5, PLL(0, 0), cost, cap, 0, true));
        graph[to].push_back(Edge(to, from, PLL(0, 0), PLL(0, 0), -cost, 0, 0, false)); //反悔边:容量0,流量0
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
        for (int t = 0; t < numMoments; ++t)
        {
            // 超级源点->user，最大流容量<边数inf, 流量demand>, 代价1，容量=demand
            for (int &u : sortedUserIds)
                addEdge(superSource, userId2GraphId[t][u], PLL(INF, demand.getDemand(t, u)), defaultCost, demand.getDemand(t, u));
            // user->site，最大流容量<边数inf, 流量INF>，代价1，容量=INF
            for (int s = 0; s < numSites; ++s)
            {
                for (int &u : siteConnUserIds[s])
                    addEdge(userId2GraphId[t][u], siteId2GraphId[t][s], PLL(INF, INF), defaultCost, INF);
            }
            // 从这里开始，图1用于最大流，图2用于费用流
            // 图1：site->site时序汇聚点，最大流容量<边数1, 带宽>；代价0，容量=0
            for (int &s : sortedSiteIds)
            {
                addEdge(siteId2GraphId[t][s], siteGatherId2GraphId[s], PLL(1, siteList.getBandwidth(s)), 0, 0);
                // 图1：site时序汇聚点->超级汇点，最大流容量<边数5%, 带宽INF>； 代价0，容量0
                if (t == 0) // 只建一次边即可
                    addEdge(siteGatherId2GraphId[s], superDest, PLL(num5moments, INF), 0, 0);
            }
            // 图2：site->汇点，拆边；最大流容量<0, 0>；默认代价1，容量之和=带宽
            for (int &s : sortedSiteIds)
            {
                int64_t bandwidth = siteList.getBandwidth(s);
                if (!is5[t][s]) // todo
                {
                    vector<int64_t> flows({bandwidth / 6,
                                           bandwidth / 3,
                                           bandwidth / 2,
                                           bandwidth});
                    for (uint32_t i = 1; i < flows.size(); ++i) // 根据阈值拆边
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

    /******************************* 使用最大流挑选5% ******************************/

    // spfa算法增广标号
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
                // 对于site时序汇聚点，来方向的余量，只有两种情况可以放行(x>0)：
                // <0,x> => 放行
                // <1,x> && 下一条边有余量 => 放行
                // 对于普通点，只有一种情况放行：
                // <随意,x>
                // 综上，不允许放行的：<h,x>, 1.x==0肯定不放行；2.h>0 && x>0 && 下一条边没有余量
                bool allow = false;
                if (PLLMinus(e.cap5, e.flow5).second > 0)
                {
                    if (graphIdType[u] == 4) // 特殊，site时序汇聚点
                    {
                        if (locks[u] == 0 || (locks[u] > 0 && PLLMinus(e.cap5, e.flow5).first > 0)) // 锁>0，钥匙>0，放行
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
                    locks[e.to] = min(locks[u], PLLMinus(e.cap5, e.flow5).first); // 最多携带的锁
                    if (!inQ[e.to] && e.to != dest)                               // 到达终点没必要继续走
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

    // 深度搜索可行流
    PLL dfsForFinding5Sites(int curNode, int dest, vector<int64_t> &level, PLL curFlowLimit)
    {
        if (curNode == dest)
            return curFlowLimit;
        PLL sonsFlowLimit = PLL(0, 0);
        for (auto &edge : graph[curNode])
        {
            if (curFlowLimit == sonsFlowLimit)
                break;
            // 放行规则，同spfaForFinding5Sites
            bool allow = false;
            if (PLLMinus(edge.cap5, edge.flow5).second > 0)
            {
                if (graphIdType[curNode] == 4) // 特殊，site时序汇聚点
                {
                    if (curFlowLimit.first == 0 || (curFlowLimit.first > 0 && PLLMinus(edge.cap5, edge.flow5).first > 0)) // 锁>0，钥匙>0，放行
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
        if (sonsFlowLimit == PLL(0, 0)) // 重要优化：此点已经没有流量，禁止后续再次访问
            level[curNode] = 0;
        return sonsFlowLimit;
    }

    // 最大流找5%
    void find5sites(int source, int dest)
    {
        static vector<int64_t> level(graph.size()); // 当前点的最短路层级
        PLL totalFlow = PLL(0, 0);                  // 总流量
        while (spfaForFinding5Sites(source, dest, level))
        {
            PLL getFlow = dfsForFinding5Sites(source, dest, level, PLL(INF, INF));
            totalFlow = PLLAdd(totalFlow, getFlow);
            cout << " -- Get flow " << getFlow.first << " " << getFlow.second << endl;
            // system("PAUSE");
        }
        if (DEBUG)
            cout << " -- Total 5% site selected: " << totalFlow.first << "; total 5% flow:" << totalFlow.second << endl;

        /****************************** 统计图上找到的5%流量，剩余的请求平分 ******************************/
        // 清空流量统计表，然后统计图上流量
        for (int t = 0; t < numMoments; ++t)
            for (int s = 0; s < numSites; ++s)
            {
                sitesFlow[t][s] = 0;
                for (int u = 0; u < numUsers; ++u)
                    flowOfUser2Site[t][s][u] = 0;
            }
        // 统计图上的流量情况
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
                        cout << "ERROR: 最大流，正向边流量为负数; " << endl;
                        cout << e.flow5.first << ":" << e.flow5.second << endl;
                        exit(7);
                    }
                    if (e.isForward && graphIdType[e.to] == 4 && e.flow5.first == 0 && e.flow5.second > 0)
                    {
                        cout << "ERROR: 最大流，时序汇聚边没解锁就放行了; " << endl;
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
        // 已分配完5%（可能不足5%），demandInfo为残余请求; 接下来给剩余的95%分配流量请求，就按平均值分发给边缘节点
        for (int t = 0; t < numMoments; ++t)
        {
            for (int &uid : sortedUserIds) // 每个用户平均分发
            {
                int64_t avgFlow = upperDiv(demandInfo[t][uid], userConnSiteIds[uid].size()); // 平均往每个边缘节点上发送的流量
                while (demandInfo[t][uid] > 0)                                               // 有可能一遍之后，请求没被处理完
                {
                    for (int &sid : userConnSiteIds[uid])
                    {
                        int flow = min(avgFlow, min((int64_t)demandInfo[t][uid], siteList.getBandwidth(sid) - sitesFlow[t][sid])); // 可行流
                        sitesFlow[t][sid] += flow;
                        flowOfUser2Site[t][sid][uid] += flow;
                        demandInfo[t][uid] -= flow;
                    }
                }
            }
        }
    }

    /******************************* 使用费用流进行全局优化 ******************************/
    // 重新指定边缘节点到超级汇点的代价；注意反悔边代价取负
    void reassignCostOfSite(int epoch)
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
            static vector<pair<int64_t, int>> sortedFlows(numMoments);
            for (int t = 0; t < numMoments; ++t)
            {
                sortedFlows[t].first = 0;
                sortedFlows[t].second = t; // 记下实际时间
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
            // 设定阈值，指定代价 todo
            double composeLower = 1 - pow(0.91, epoch); // (1-adjustRange -> 1.0) 影响巨大
            double composeUpper = 1 + pow(0.9, epoch);  // 影响微小
            vector<int64_t> threshold({0,
                                       //    (int64_t)(sortedFlows[num95moments - 1].first / 2),
                                       (int64_t)(sortedFlows[num95moments - 1].first * composeLower),
                                       sortedFlows[num95moments - 1].first,
                                       //    sortedFlows[num95moments].first,
                                       min((int64_t)(sortedFlows[num95moments - 1].first * composeUpper), (int64_t)siteList.getBandwidth(siteId)),
                                       siteList.getBandwidth(siteId)}); // 阈值
            // 修改边代价
            for (auto &flowOfmoment : sortedFlows)
            {
                int level = 0;
                int moment = flowOfmoment.second;
                for (Edge &edge : graph[siteId2GraphId[moment][siteId]])
                    if (edge.isForward)
                    {
                        ++level;
                        // if (!is5[moment][siteId]) // 小于96; 分4条边 todo
                        if (flowOfmoment.first <= sortedFlows[num95moments - 1].first) // 小于96; 分4条边
                            resetCostOfEdge(edge, level, threshold[level] - threshold[level - 1]);
                        else // 最后5%直接满载，代价最低
                        {
                            if (level == 1)
                                resetCostOfEdge(edge, 1, siteList.getBandwidth(siteId));
                            else
                                resetCostOfEdge(edge, 0, 0);
                        }
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
        if (sonsFlowLimit == 0) // 重要优化：此点已经没有流量，禁止后续再次访问
            level[curNode] = 0;
        return sonsFlowLimit;
    }

    // zkw算法求解最小费用流
    void mincost(int source, int dest)
    {
        static vector<int64_t> cost(graph.size()); // 当前点的花费
        // static vector<int64_t> flow(graph.size());   // 当前点的流量
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
            cout << " -- Total flow " << totalFlow << "; Total cost " << totalCost << endl;
    }

    /******************************** 统计信息 ***********************************/
    // 统计费用流图上流量
    void gatherGraphFlow()
    {
        // 清空流量统计表，然后统计图上流量
        for (int t = 0; t < numMoments; ++t)
            for (int s = 0; s < numSites; ++s)
            {
                sitesFlow[t][s] = 0;
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
                        sitesFlow[moment][siteId] += e.flow;          // 用于计算成本
                    }
                }
            }
        }
    }

    // 计算总成本 95计费
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

    // 将答案输出到文件
    void output(string filepath)
    {
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
        cout << "Solution has been written to file " << filepath << endl;
    }

    /******************************** 解题整体逻辑 ********************************/
    void run(int epochs = 50)
    {
        auto startClock = clock();
        buildGraph();
        /************************** 最大流挑选5% *************************/
        find5sites(superSource, superDest);
        return;
        /************************* 费用流全局优化 ************************/
        mincost(superSource, superDest); // 初始跑一个解
        for (int epoch = 0; epoch < epochs; epoch++)
        {
            auto epochClock = clock();
            if (DEBUG)
                cout << "Epoch " << epoch << ": " << endl;
            reassignCostOfSite(epoch); // 先将流量清零，并设置代价
            mincost(superSource, superDest);
            if (DEBUG)
            {
                gatherGraphFlow();
                cout << " -- Epoch Time Spent " << (clock() - epochClock) << "ms. Problem Cost " << calclateProblemCost() << endl;
            }
            if ((clock() - startClock) / 1000000 > 290)
                break;
        }
        gatherGraphFlow(); // 费用流结束后统计图上流量
        /***************************** 策略结束 *****************************/
        if (DEBUG)
            cout << "Total time spent " << (clock() - startClock) << endl;
    }
};

int main()
{
    /******** 确定数据集路径 ********/
    string dataDir = "/data", outputPath = "/output/solution.txt"; // 默认线上路径
    string linuxDir = "../dataset/data";                           // linux测试路径
    // string linuxDir = "../dataset/data200"; // linux测试路径
    // string linuxDir = "../dataset/pressure0.6_data"; // linux测试路径
    // string linuxDir = "../dataset/pressure0.3-t8926-n135-m35"; // linux测试路径
    // string linuxDir = "../dataset/pressure0.3-t200-n100-m20"; // linux测试路径
    string winDir = "../" + linuxDir;             // win测试路径
    if (ifstream(dataDir + "/config.ini").good()) // 线上环境
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
