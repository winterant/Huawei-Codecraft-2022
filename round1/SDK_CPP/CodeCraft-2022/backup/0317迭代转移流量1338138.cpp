#include <bits/stdc++.h>
using namespace std;

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

    // 流量
    vector<vector<vector<int>>> userFlow; // userFlow[t][s][u]表示t时刻节点s与用户u之间的流量
    vector<vector<int>> siteFlow;         // siteFlow[t][s]表示t时刻s节点上的总流量

    // 堆; 为每个边缘节点维护两个关于时间的堆: 大顶堆heap95,小顶堆heap5
    typedef pair<int, int> PII; // pair[first:流量值,second:时刻]
    vector<priority_queue<PII, vector<PII>, less<PII>>> heap95;
    vector<priority_queue<PII, vector<PII>, greater<PII>>> heap5;
    vector<vector<int>> siteInWhichHeap; // [s][t]: 边缘节点s在t时刻属于哪一个heap

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
        // 初始化堆的个数
        heap95.resize(numSites);
        heap5.resize(numSites);
        siteInWhichHeap.resize(numSites);
        for (auto &i : siteInWhichHeap)
            i.resize(numMoments);
    }

private:
    // 增加流量：t时刻节点s与用户u之间增加流量bandwidth
    void add(int moment, int siteId, int userId, int bandwidth)
    {
        userFlow[moment][siteId][userId] += bandwidth;
        siteFlow[moment][siteId] += bandwidth;
        // if(userFlow[moment][siteId]<0||userFlow[moment][siteId]>siteList.getBandwidth(siteId))
    }

    // 转移流量：t时刻用户u到节点s的流量bandwidth转移到target_siteId上
    void transferUserFlow(int moment, int siteId, int userId, int target_siteId, int bandwidth)
    {
        add(moment, siteId, userId, -bandwidth);
        add(moment, target_siteId, userId, bandwidth);
    }

    // 初始化堆，必须获得任意可行解之后进行
    void resetHeap(int siteId)
    {
        // 先清空堆
        while (!heap95[siteId].empty())
            heap95[siteId].pop();
        while (!heap5[siteId].empty())
            heap5[siteId].pop();
        // 先全部放入大顶堆heap95
        for (int t = 0; t < numMoments; ++t)
        {
            heap95[siteId].push(PII(siteFlow[t][siteId], t));
            siteInWhichHeap[siteId][t] = 95;
        }
        // 取出流量最大的5%放入小顶堆heap5
        for (int i = 0; i < num5moments; ++i)
        {
            auto top95 = heap95[siteId].top();
            heap95[siteId].pop();
            heap5[siteId].push(top95);
            siteInWhichHeap[siteId][top95.second] = 5;
        }
    }

    // 给定堆heap，获取heap的堆顶元素
    template <class HeapVec>
    PII getHeapTop(HeapVec &heapVec, int siteId, bool pop = false)
    {
        auto &heap = heapVec[siteId];
        while (!heap.empty() && heap.top().first != siteFlow[heap.top().second][siteId])
            heap.pop();
        if (heap.empty())
        {
            cout << "Can't get the top element of a empty heap!" << endl;
            exit(5);
        }
        PII ret = heap.top();
        if (pop)
            heap.pop();
        return ret;
    }

    // 调整大顶堆和小顶堆，使heap95堆顶<=heap5堆顶
    void adjustHeaps(int siteId)
    {
        while (true)
        {
            auto top95 = getHeapTop(heap95, siteId);
            auto top5 = getHeapTop(heap5, siteId);
            if (top95.first > top5.first)
            {
                heap95[siteId].pop();
                heap5[siteId].pop();
                heap95[siteId].push(top5);
                heap5[siteId].push(top95);
                siteInWhichHeap[siteId][top5.second] = 95;
                siteInWhichHeap[siteId][top95.second] = 5;
            }
            else
                break;
        }
    }

    // 计算当前状态的总成本
    int calculateCost()
    {
        int cost = 0;
        for (int s = 0; s < numSites; ++s)
            cost += getHeapTop(heap95, s).first;
        return cost;
    }

public:
    int run(int epochs = 1000, int maxHeapSize = 10000)
    {
        /**
         * 核心思路：
         * 先任意初始化一个可行解，然后通过调整用户流量来优化95百分位的带宽值
         * 对于一个边缘节点s，按带宽值将时间序列排序，记第95%时刻为t，其流量值为flow
         * 优化目标：min{flow}
         *   1. 取前95%带宽值取均值，记为avg95
         *   2. 将s在t时刻的流量减掉trans={flow-avg95, 最大可转移流量}
         *   3. 将trans转移到其它可连接边缘节点，转移方式待考虑：
         *      a. 平均分配
         *      b. 带权转移到其它节点，权重可参考其它节点的95百分位带宽大小
         * 收敛条件：trans=0
         **/
        // 1. 初始化一个可行解
        for (int t = 0; t < numMoments; ++t)
        {
            vector<int> band_capacity(numSites); // 当前时刻边缘节点s剩余容量
            for (int s = 0; s < numSites; ++s)
                band_capacity[s] = siteList.getBandwidth(s);

            // 为每个用户分配流量
            for (int u = 0; u < numUsers; ++u)
            {
                int cur_demand = demand.getDemand(t, u); // 当前时刻用户u的需求量
                for (int s = 0; s < numSites; ++s)
                {
                    if (cur_demand <= 0)
                        break;
                    if (band_capacity[s] == 0 || qos.get(s, u) >= qosConstraint)
                        continue;
                    int bandwidth = min(cur_demand, band_capacity[s]);
                    add(t, s, u, bandwidth);
                    cur_demand -= bandwidth;
                    band_capacity[s] -= bandwidth;
                }
            }
        }

        // 2. 初始化堆
        for (int s = 0; s < numSites; ++s)
            resetHeap(s);

        // 3. 迭代边缘节点，优化95%节点的流量
        for (int epoch = 0; epoch < epochs; ++epoch)
        {
            for (int siteId = 0; siteId < numSites; ++siteId)
            {
                int sumFlowOf95 = 0;
                for (int t = 0; t < numMoments; ++t)
                {
                    if (siteInWhichHeap[siteId][t] == 95)
                        sumFlowOf95 += siteFlow[t][siteId];
                }
                PII state = getHeapTop(heap95, siteId, true); // 取出95百分位对应时刻
                int currentSiteFlow = state.first;            // 95百分位时刻对应的边缘节点实际流量
                int moment = state.second;                    // 95百分位所处的时刻
                // int transferredFlow = max(1000, currentSiteFlow - sumFlowOf95 / num95moments); // 当前边缘节点需要转移走的流量; todo
                int transferredFlow = max(1, currentSiteFlow / 80); // 当前边缘节点需要转移走的流量; todo
                if (transferredFlow < 0)
                {
                    cout << "Flow to be transferred can't less than 0." << endl;
                    exit(6);
                }

                vector<PII> connectedUsers; // 保存与当前边缘节点相连的用户流量信息
                set<int> changedSiteIds;    // 保存流量发生过变化的边缘节点

                // 开始收集用户信息
                for (int userId = 0; userId < numUsers; ++userId)
                {
                    if (userFlow[moment][siteId][userId] > 0)
                        connectedUsers.push_back(PII(userId, userFlow[moment][siteId][userId]));
                }

                // 对于每个用户userId，将它的流量转移到其它边缘节点
                changedSiteIds.insert(siteId);
                for (auto &userInfo : connectedUsers)
                {
                    int userId = userInfo.first;
                    int currentUserFlow = userInfo.second;                                                              // userId与siteId之间的实时流量
                    int reducible = min(currentUserFlow, transferredFlow * currentUserFlow / siteFlow[moment][siteId]); // userId按比例回退的流量
                    // 遍历其它边缘节点nextSiteId，请求它们接收流量; 将user的流量reducible分发给它们
                    for (int nextSiteId = 0; nextSiteId < numSites; ++nextSiteId)
                    {
                        // siteId自身跳过，时延超时的跳过
                        if (nextSiteId == siteId || qos.get(nextSiteId, userId) >= qosConstraint)
                            continue;
                        int acceptable = siteList.getBandwidth(nextSiteId) - siteFlow[moment][nextSiteId]; // nextSiteId可接收量
                        int transferable = min(reducible, acceptable);
                        if (transferable > 0)
                        {
                            transferUserFlow(moment, siteId, userId, nextSiteId, transferable);
                            reducible -= transferable;
                            changedSiteIds.insert(nextSiteId);
                        }
                    }
                }
                // 流量发生了变化的边缘节点，要更新堆
                for (auto changedId : changedSiteIds)
                {
                    // 哪个堆里的元素更新了，就压入哪个堆，堆内原来的相同元素就作废了
                    if (siteInWhichHeap[changedId][moment] == 95)
                        heap95[changedId].push(PII(siteFlow[moment][changedId], moment));
                    else
                        heap5[changedId].push(PII(siteFlow[moment][changedId], moment));
                }
                // 调整大小堆的平衡状态
                for (auto changedId : changedSiteIds)
                    adjustHeaps(changedId);
                // 重置过于臃肿的堆
                for (auto changedId : changedSiteIds)
                    if (heap95[changedId].size() + heap5[changedId].size() > (unsigned)maxHeapSize)
                        resetHeap(changedId);
            }
            cout << "Epoch " << epoch << " cost:" << calculateCost() << endl;
        }

        // 4. 评估成本
        return calculateCost();
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
