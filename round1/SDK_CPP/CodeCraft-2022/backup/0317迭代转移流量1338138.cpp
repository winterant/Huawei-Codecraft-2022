#include <bits/stdc++.h>
using namespace std;

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

    // ����
    vector<vector<vector<int>>> userFlow; // userFlow[t][s][u]��ʾtʱ�̽ڵ�s���û�u֮�������
    vector<vector<int>> siteFlow;         // siteFlow[t][s]��ʾtʱ��s�ڵ��ϵ�������

    // ��; Ϊÿ����Ե�ڵ�ά����������ʱ��Ķ�: �󶥶�heap95,С����heap5
    typedef pair<int, int> PII; // pair[first:����ֵ,second:ʱ��]
    vector<priority_queue<PII, vector<PII>, less<PII>>> heap95;
    vector<priority_queue<PII, vector<PII>, greater<PII>>> heap5;
    vector<vector<int>> siteInWhichHeap; // [s][t]: ��Ե�ڵ�s��tʱ��������һ��heap

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
        // ��ʼ���ѵĸ���
        heap95.resize(numSites);
        heap5.resize(numSites);
        siteInWhichHeap.resize(numSites);
        for (auto &i : siteInWhichHeap)
            i.resize(numMoments);
    }

private:
    // ����������tʱ�̽ڵ�s���û�u֮����������bandwidth
    void add(int moment, int siteId, int userId, int bandwidth)
    {
        userFlow[moment][siteId][userId] += bandwidth;
        siteFlow[moment][siteId] += bandwidth;
        // if(userFlow[moment][siteId]<0||userFlow[moment][siteId]>siteList.getBandwidth(siteId))
    }

    // ת��������tʱ���û�u���ڵ�s������bandwidthת�Ƶ�target_siteId��
    void transferUserFlow(int moment, int siteId, int userId, int target_siteId, int bandwidth)
    {
        add(moment, siteId, userId, -bandwidth);
        add(moment, target_siteId, userId, bandwidth);
    }

    // ��ʼ���ѣ�������������н�֮�����
    void resetHeap(int siteId)
    {
        // ����ն�
        while (!heap95[siteId].empty())
            heap95[siteId].pop();
        while (!heap5[siteId].empty())
            heap5[siteId].pop();
        // ��ȫ������󶥶�heap95
        for (int t = 0; t < numMoments; ++t)
        {
            heap95[siteId].push(PII(siteFlow[t][siteId], t));
            siteInWhichHeap[siteId][t] = 95;
        }
        // ȡ����������5%����С����heap5
        for (int i = 0; i < num5moments; ++i)
        {
            auto top95 = heap95[siteId].top();
            heap95[siteId].pop();
            heap5[siteId].push(top95);
            siteInWhichHeap[siteId][top95.second] = 5;
        }
    }

    // ������heap����ȡheap�ĶѶ�Ԫ��
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

    // �����󶥶Ѻ�С���ѣ�ʹheap95�Ѷ�<=heap5�Ѷ�
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

    // ���㵱ǰ״̬���ܳɱ�
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
         * ����˼·��
         * �������ʼ��һ�����н⣬Ȼ��ͨ�������û��������Ż�95�ٷ�λ�Ĵ���ֵ
         * ����һ����Ե�ڵ�s��������ֵ��ʱ���������򣬼ǵ�95%ʱ��Ϊt��������ֵΪflow
         * �Ż�Ŀ�꣺min{flow}
         *   1. ȡǰ95%����ֵȡ��ֵ����Ϊavg95
         *   2. ��s��tʱ�̵���������trans={flow-avg95, ����ת������}
         *   3. ��transת�Ƶ����������ӱ�Ե�ڵ㣬ת�Ʒ�ʽ�����ǣ�
         *      a. ƽ������
         *      b. ��Ȩת�Ƶ������ڵ㣬Ȩ�ؿɲο������ڵ��95�ٷ�λ�����С
         * ����������trans=0
         **/
        // 1. ��ʼ��һ�����н�
        for (int t = 0; t < numMoments; ++t)
        {
            vector<int> band_capacity(numSites); // ��ǰʱ�̱�Ե�ڵ�sʣ������
            for (int s = 0; s < numSites; ++s)
                band_capacity[s] = siteList.getBandwidth(s);

            // Ϊÿ���û���������
            for (int u = 0; u < numUsers; ++u)
            {
                int cur_demand = demand.getDemand(t, u); // ��ǰʱ���û�u��������
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

        // 2. ��ʼ����
        for (int s = 0; s < numSites; ++s)
            resetHeap(s);

        // 3. ������Ե�ڵ㣬�Ż�95%�ڵ������
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
                PII state = getHeapTop(heap95, siteId, true); // ȡ��95�ٷ�λ��Ӧʱ��
                int currentSiteFlow = state.first;            // 95�ٷ�λʱ�̶�Ӧ�ı�Ե�ڵ�ʵ������
                int moment = state.second;                    // 95�ٷ�λ������ʱ��
                // int transferredFlow = max(1000, currentSiteFlow - sumFlowOf95 / num95moments); // ��ǰ��Ե�ڵ���Ҫת���ߵ�����; todo
                int transferredFlow = max(1, currentSiteFlow / 80); // ��ǰ��Ե�ڵ���Ҫת���ߵ�����; todo
                if (transferredFlow < 0)
                {
                    cout << "Flow to be transferred can't less than 0." << endl;
                    exit(6);
                }

                vector<PII> connectedUsers; // �����뵱ǰ��Ե�ڵ��������û�������Ϣ
                set<int> changedSiteIds;    // ���������������仯�ı�Ե�ڵ�

                // ��ʼ�ռ��û���Ϣ
                for (int userId = 0; userId < numUsers; ++userId)
                {
                    if (userFlow[moment][siteId][userId] > 0)
                        connectedUsers.push_back(PII(userId, userFlow[moment][siteId][userId]));
                }

                // ����ÿ���û�userId������������ת�Ƶ�������Ե�ڵ�
                changedSiteIds.insert(siteId);
                for (auto &userInfo : connectedUsers)
                {
                    int userId = userInfo.first;
                    int currentUserFlow = userInfo.second;                                                              // userId��siteId֮���ʵʱ����
                    int reducible = min(currentUserFlow, transferredFlow * currentUserFlow / siteFlow[moment][siteId]); // userId���������˵�����
                    // ����������Ե�ڵ�nextSiteId���������ǽ�������; ��user������reducible�ַ�������
                    for (int nextSiteId = 0; nextSiteId < numSites; ++nextSiteId)
                    {
                        // siteId����������ʱ�ӳ�ʱ������
                        if (nextSiteId == siteId || qos.get(nextSiteId, userId) >= qosConstraint)
                            continue;
                        int acceptable = siteList.getBandwidth(nextSiteId) - siteFlow[moment][nextSiteId]; // nextSiteId�ɽ�����
                        int transferable = min(reducible, acceptable);
                        if (transferable > 0)
                        {
                            transferUserFlow(moment, siteId, userId, nextSiteId, transferable);
                            reducible -= transferable;
                            changedSiteIds.insert(nextSiteId);
                        }
                    }
                }
                // ���������˱仯�ı�Ե�ڵ㣬Ҫ���¶�
                for (auto changedId : changedSiteIds)
                {
                    // �ĸ������Ԫ�ظ����ˣ���ѹ���ĸ��ѣ�����ԭ������ͬԪ�ؾ�������
                    if (siteInWhichHeap[changedId][moment] == 95)
                        heap95[changedId].push(PII(siteFlow[moment][changedId], moment));
                    else
                        heap5[changedId].push(PII(siteFlow[moment][changedId], moment));
                }
                // ������С�ѵ�ƽ��״̬
                for (auto changedId : changedSiteIds)
                    adjustHeaps(changedId);
                // ���ù���ӷ�׵Ķ�
                for (auto changedId : changedSiteIds)
                    if (heap95[changedId].size() + heap5[changedId].size() > (unsigned)maxHeapSize)
                        resetHeap(changedId);
            }
            cout << "Epoch " << epoch << " cost:" << calculateCost() << endl;
        }

        // 4. �����ɱ�
        return calculateCost();
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
