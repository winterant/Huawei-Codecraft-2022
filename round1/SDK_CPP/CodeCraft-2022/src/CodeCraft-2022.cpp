#include <bits/stdc++.h>
using namespace std;

// 向上取整的除法
int64_t upperDiv(int64_t num, int64_t div)
{
    return (num + div - 1) / div;
}

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
    int constraint;

public:
    Qos(string filepath, SiteList &siteList, Demand &demand, int qosConstraint)
    {
        constraint = qosConstraint;
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

    bool isConnected(int siteId, int userId)
    {
        return qos[siteId][userId] < constraint;
    }
};

// 解决方案
class Solution
{
    typedef pair<int, int> PII;
    typedef pair<int64_t, int> PLI;
    const int64_t INF64 = (1LL << 40);

private:
    bool DEBUG; // 标记是否线下

    /********************** 数据集信息处理 *****************************/
    // 数据集
    SiteList &siteList; //边缘节点集
    Demand &demand;     //需求集
    Qos &qos;           //时延表

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

    /******************************** 图 *******************************/
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

    /************************************ 统计流量 **************************************/
    vector<vector<int64_t>> sitesFlow;               // [t][s]表示t时刻s节点上的总流量
    vector<vector<vector<int64_t>>> flowOfUser2Site; // 统计用户到边缘节点的流量

public:
    Solution(SiteList &siteList, Demand &demand, Qos &qos, bool debug = false) : DEBUG(debug), siteList(siteList), demand(demand), qos(qos)
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
                if (qos.isConnected(s, u))
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
            sort(userConnSiteIds[u].begin(), userConnSiteIds[u].end(),
                 [=](int i, int j)
                 {
                     if (siteConnUserIds[i].size() == siteConnUserIds[j].size())
                         return this->siteList.getBandwidth(i) > this->siteList.getBandwidth(j);
                     return siteConnUserIds[i].size() < siteConnUserIds[j].size();
                 });
        for (int s = 0; s < numSites; ++s)
            if (siteConnUserIds[s].size() > 0)
                sortedSiteIds.push_back(s);
        for (int u = 0; u < numUsers; ++u)
            if (userConnSiteIds[u].size() > 0)
                sortedUserIds.push_back(u);
        sort(sortedSiteIds.begin(), sortedSiteIds.end(),
             [=](int i, int j)
             {
                 if (siteConnUserIds[i].size() == siteConnUserIds[j].size())
                     return this->siteList.getBandwidth(i) > this->siteList.getBandwidth(j);
                 return siteConnUserIds[i].size() < siteConnUserIds[j].size();
             }); //按可连接用户的数量排序
        sort(sortedUserIds.begin(), sortedUserIds.end(), [=](int i, int j)
             { return userConnSiteIds[i].size() < userConnSiteIds[j].size(); }); //按照可连接边缘节点的数量排序

        /****************************** 初始化图内存 *****************************/
        graph.resize(2 + numMoments * (numUsers + numSites)); // 图上节点的个数：2个源汇点，user/site各复制T倍
        userId2GraphId.resize(numMoments);
        siteId2GraphId.resize(numMoments);
        for (int t = 0; t < numMoments; ++t)
        {
            userId2GraphId[t].resize(numUsers);
            siteId2GraphId[t].resize(numSites);
        }
        graphId2ServeId.resize(graph.size());

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

    // 贪心：每个边缘节点选一个时刻，反复5次，选出5%
    void greed5()
    {
        auto demandInfo = demand.cloneDemand(); // [moment][userId]
        // 每个用户可连接的带宽余量之和
        vector<vector<int64_t>> userSumAllowedBandwidth(numMoments, vector<int64_t>(numUsers, 0));
        for (int t = 0; t < numMoments; ++t)
            for (int u = 0; u < numUsers; ++u)
                for (int &sid : userConnSiteIds[u])
                    userSumAllowedBandwidth[t][u] += siteList.getBandwidth(sid);
        // 每个时间点 每个边缘节点潜力值
        vector<vector<int64_t>> potential(numMoments, vector<int64_t>(numSites, 0));
        for (int t = 0; t < numMoments; ++t)
        {
            for (int s : sortedSiteIds)
                for (int u : siteConnUserIds[s])
                    potential[t][s] += (int64_t)demandInfo[t][u] * siteList.getBandwidth(s) / userSumAllowedBandwidth[t][u];
        }
        // 开始选取5%边缘节点
        vector<int> siteNumOptionalMoments(numSites, num5moments); // 每个节点剩余可选时刻数
        int numOptionalSites = sortedSiteIds.size();               // 未处理完成的边缘节点的剩余数量
        while (numOptionalSites)
        {
            for (int &sid : sortedSiteIds)
            {
                if (siteNumOptionalMoments[sid] == 0)
                    continue;
                uint32_t numMomentsToBeChose = max(1, min(siteNumOptionalMoments[sid], num5moments / 1)); // 本轮要选出的5%节点个数
                siteNumOptionalMoments[sid] -= numMomentsToBeChose;
                if (siteNumOptionalMoments[sid] == 0)
                    --numOptionalSites;
                // 当前节点选出numMomentsToBeChose个最优时刻
                // priority_queue<PLI, vector<PLI>, greater<PLI>> Qband; // 策略1（小顶堆）
                priority_queue<PLI> Qband; // 策略2/3（大顶堆）
                for (int t = 0; t < numMoments; ++t)
                {
                    // Qband.push(PLI(curSumFlow, t));
                    Qband.push(PLI(abs(potential[t][sid] - siteList.getBandwidth(sid)), t)); // 策略2
                    while (Qband.size() > numMomentsToBeChose)
                        Qband.pop();
                }
                // 选出了若干时刻节点，将其拉满
                unordered_set<int> selectedMoments; // 选中的时间
                while (!Qband.empty())
                {
                    int bestMoment = Qband.top().second;
                    Qband.pop();
                    selectedMoments.insert(bestMoment);
                    // 选好了时刻bestMoment，该时刻sid可以填充: uid -> sid
                    int64_t incrementFlow = 0;
                    // 第一次接收请求，按需求比例接收
                    int64_t sumUserDemand = 1;
                    for (int &uid : siteConnUserIds[sid])
                        sumUserDemand += demandInfo[bestMoment][uid];
                    for (int &uid : siteConnUserIds[sid])
                    {
                        int64_t flow = (siteList.getBandwidth(sid) - sitesFlow[bestMoment][sid]) * demandInfo[bestMoment][uid] / sumUserDemand;
                        demandInfo[bestMoment][uid] -= flow;
                        sitesFlow[bestMoment][sid] += flow;
                        flowOfUser2Site[bestMoment][sid][uid] += flow;
                        incrementFlow += flow;
                    }
                    // 第二次贪心接收请求
                    for (int &uid : siteConnUserIds[sid])
                    {
                        // trick: 用户请求不要全发给一个边缘节点，可以优化
                        int64_t flow = min((int64_t)demandInfo[bestMoment][uid], siteList.getBandwidth(sid) - sitesFlow[bestMoment][sid]);
                        demandInfo[bestMoment][uid] -= flow;
                        sitesFlow[bestMoment][sid] += flow;
                        flowOfUser2Site[bestMoment][sid][uid] += flow;
                        incrementFlow += flow;
                    }
                    // 刷新用户可连接带宽余量总和
                    for (int &uid : siteConnUserIds[sid])
                        userSumAllowedBandwidth[bestMoment][uid] -= incrementFlow;
                }
                // 更新潜力表, 对于有更新的时间点
                for (auto t : selectedMoments)
                {
                    for (int s : sortedSiteIds)
                    {
                        potential[t][s] = 0;
                        for (int u : siteConnUserIds[s])
                            potential[t][s] += demandInfo[t][u] * (siteList.getBandwidth(s) - sitesFlow[t][s]) / userSumAllowedBandwidth[t][u];
                    }
                }
            }
        }
        // 已分配完5%（可能不足5%），将demandInfo残余请求分发到边缘节点（95%）
        // 按剩余带宽比例
        for (int t = 0; t < numMoments; ++t)
        {
            for (int &uid : sortedUserIds) // 每个用户平均分发
            {
                if (demandInfo[t][uid] == 0) // 没有请求了，还分发个鸡毛
                    continue;
                // 先算出剩余容量的概率分布
                static vector<int64_t> remainBand(numSites);
                int64_t sumRemainBand = 0;
                for (int s = 0; s < numSites; ++s)
                {
                    if (qos.isConnected(s, uid))
                    {
                        remainBand[s] = siteList.getBandwidth(s) - sitesFlow[t][s];
                        sumRemainBand += remainBand[s];
                    }
                }
                // 开始分发
                const int curUserDemand = demandInfo[t][uid];
                for (int &sid : userConnSiteIds[uid])
                {
                    int64_t planFlow = upperDiv(curUserDemand * remainBand[sid], sumRemainBand);           // 计划分配流量
                    int64_t actualFlow = min((int64_t)demandInfo[t][uid], min(planFlow, remainBand[sid])); // 其实计划流量肯定能流过去，这句没必要
                    demandInfo[t][uid] -= actualFlow;
                    sitesFlow[t][sid] += actualFlow;
                    flowOfUser2Site[t][sid][uid] += actualFlow;
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

    // 建图，跑网络流优化
    void buildGraph(int64_t defaultCost = 1)
    {
        // 根据贪心算出的流量分配情况，计算出每个边缘节点的时序流量
        vector<vector<PLI>> siteTimedFlow(numSites, vector<PLI>(numMoments, PLI(0, 0))); // <flow,moment>
        // vector<int64_t> siteSum95Flow(numSites);                                         // 前95的流量之和
        vector<vector<bool>> is5(numMoments, vector<bool>(numSites, false));
        for (int s = 0; s < numSites; ++s)
        {
            for (int t = 0; t < numMoments; ++t)
            {
                siteTimedFlow[s][t].first = sitesFlow[t][s];
                siteTimedFlow[s][t].second = t;
            }
            sort(siteTimedFlow[s].begin(), siteTimedFlow[s].end());
            // siteSum95Flow[s] = 0;
            // for (int i = 0; i < num95moments; i++)
            // siteSum95Flow[s] += siteTimedFlow[s][i].first;
            for (int i = num95moments; i < numMoments; i++)
                is5[siteTimedFlow[s][i].second][s] = true; // 标记5%点
        }
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
            for (int &u : sortedUserIds)
                addEdge(superSource, userId2GraphId[t][u], defaultCost, demand.getDemand(t, u));
            // user->site，代价1，容量=INF
            for (int s = 0; s < numSites; ++s)
            {
                for (int &u : siteConnUserIds[s])
                    addEdge(userId2GraphId[t][u], siteId2GraphId[t][s], defaultCost, INF64);
            }
            // site->汇点，拆边，默认代价1，容量之和=带宽
            for (int &s : sortedSiteIds)
            {
                vector<int64_t> threshold({siteTimedFlow[s][num95moments - 1].first * 40 / 100,
                                           siteTimedFlow[s][num95moments - 1].first * 70 / 100,
                                           siteTimedFlow[s][num95moments - 1].first,
                                           siteList.getBandwidth(s)});
                sort(threshold.begin(), threshold.end());
                for (uint32_t i = 0; i < threshold.size(); ++i)
                {
                    threshold[i] = max(threshold[i], (int64_t)0);
                    threshold[i] = min(threshold[i], (int64_t)siteList.getBandwidth(s));
                    if (i > 0)
                        threshold[i] -= threshold[i - 1];
                }
                if (!is5[t][s]) // 普通节点拆边
                {
                    for (uint32_t i = 0; i < threshold.size(); ++i)
                        addEdge(siteId2GraphId[t][s], superDest, defaultCost * (i + 1), threshold[i]);
                }
                else
                {
                    addEdge(siteId2GraphId[t][s], superDest, 0, siteList.getBandwidth(s));
                    addEdge(siteId2GraphId[t][s], superDest, 0, 0);
                    addEdge(siteId2GraphId[t][s], superDest, 0, 0);
                    addEdge(siteId2GraphId[t][s], superDest, 0, 0);
                }
            }
        }
        cout << "Meked Graph with " << graph.size() << " nodes." << endl;
    }

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
                        sortedFlows[t].first += edge.flow;
                }
            }
            sort(sortedFlows.begin(), sortedFlows.end());
            // 设定阈值，指定代价 todo
            double composeLower = 1 - pow(0.91, epoch + 1); // (1-adjustRange -> 1.0) 影响巨大
            double composeUpper = 1 + pow(0.9, epoch + 1);  // 影响微小
            vector<int64_t> threshold({(int64_t)(sortedFlows[num95moments - 1].first * composeLower),
                                       //    sortedFlows[num95moments - 1].first * 50 / 100,
                                       sortedFlows[num95moments - 1].first,
                                       //    sortedFlows[num95moments - 1].first * 80 / 100,
                                       min((int64_t)(sortedFlows[num95moments - 1].first * composeUpper), (int64_t)siteList.getBandwidth(siteId)),
                                       siteList.getBandwidth(siteId)}); // 阈值
            sort(threshold.begin(), threshold.end());
            for (uint32_t i = 0; i < threshold.size(); ++i)
            {
                threshold[i] = max(threshold[i], (int64_t)0);
                threshold[i] = min(threshold[i], (int64_t)siteList.getBandwidth(siteId));
                if (i > 0)
                    threshold[i] -= threshold[i - 1];
            }
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
                            resetCostOfEdge(edge, level, threshold[level - 1]);
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
            cost[i] = INF64;
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
        return cost[dest] < INF64;
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
                getFlow = dfs(source, dest, cost, level, INF64, totalCost);
                totalFlow += getFlow;
            }
        }
        // if (DEBUG)
        //     cout << " -- Total flow " << totalFlow << "; Total cost " << totalCost << endl;
    }

    // 统计网络流图上流量
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
        cout << "Solution has been wrote to file " << filepath << endl;
    }

    void run(int epochs = 250)
    {
        auto startClock = clock();
        /************************** 贪心策略 *************************/
        greed5();
        cout << "Greedy Cost: " << calclateProblemCost() << endl;
        // return;
        /************************* 网络流策略 ************************/
        buildGraph();
        mincost(superSource, superDest); // 初始跑一个解，reassignCostOfSite才能工作
        if (DEBUG)
        {
            gatherGraphFlow();
            cout << " -- init. mincost.  Problem Cost " << calclateProblemCost() << endl;
        }
        for (int epoch = 0; epoch < epochs; epoch++)
        {
            auto epochClock = clock();
            reassignCostOfSite(epoch); // 先将流量清零，并设置代价
            mincost(superSource, superDest);
            if (DEBUG)
            {
                gatherGraphFlow();
                cout << " -- Epoch " << epoch << "; Time Spent " << (clock() - epochClock) << "ms. Problem Cost " << calclateProblemCost() << endl;
            }
            if ((clock() - startClock) / 1000000 > 295)
                break;
        }
        gatherGraphFlow();

        if (DEBUG)
            cout << "Total time spent " << (clock() - startClock) << endl;
    }
};

int main()
{
    /******** 确定数据集路径 ********/
    bool debug = true;
    string dataDir = "/data", outputPath = "/output/solution.txt"; // 默认线上路径
    string linuxDir = "../dataset/data";                           // linux测试路径
    // string linuxDir = "../dataset/data200"; // linux测试路径
    // string linuxDir = "../dataset/pressure0.6_data"; // linux测试路径
    // string linuxDir = "../dataset/pressure0.3-t8926-n135-m35"; // linux测试路径
    // string linuxDir = "../dataset/pressure0.3-t200-n100-m20"; // linux测试路径
    string winDir = "../" + linuxDir;             // win测试路径
    if (ifstream(dataDir + "/config.ini").good()) // 线上环境
    {
        debug = false;
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

    /********* 读取数据集 *********/
    int qosConstraint = atoi(getConfigIni(dataDir + "/config.ini", "config", "qos_constraint").c_str());
    SiteList siteList(dataDir + "/site_bandwidth.csv");
    Demand demand(dataDir + "/demand.csv");
    Qos qos(dataDir + "/qos.csv", siteList, demand, qosConstraint);

    /********* 执行解决方案 *********/
    Solution solution(siteList, demand, qos, debug);
    solution.run();

    /********* 将结果写入文件 *********/
    solution.output(outputPath);
    return 0;
}
