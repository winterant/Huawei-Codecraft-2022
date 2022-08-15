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

    // ʵʱ����
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
    }

    // ת��������tʱ���û�u���ڵ�s������bandwidthת�Ƶ�target_siteId��
    void transferUserFlow(int moment, int siteId, int userId, int target_siteId, int bandwidth)
    {
        add(moment, siteId, userId, -bandwidth);
        add(moment, target_siteId, userId, bandwidth);
    }

    // ��ʼ����(���ö�)
    void resetHeap(int siteId)
    {
        // ��ն�
        while (!heap95[siteId].empty())
            heap95[siteId].pop();
        while (!heap5[siteId].empty())
            heap5[siteId].pop();
        // ��num95Moment����ʱ�ڵ����󶥶�heap95���������С����heap5
        for (int t = 0; t < numMoments; ++t)
        {
            heap95[siteId].push(PII(siteFlow[t][siteId], t));
            siteInWhichHeap[siteId][t] = 95;
            // ��heap95Ԫ������(t+1)����ʱ���Ѷ�Ӧ������heap5
            if (t + 1 > num95moments)
            {
                auto top95 = heap95[siteId].top();
                heap95[siteId].pop();
                heap5[siteId].push(top95);
                siteInWhichHeap[siteId][top95.second] = 5;
            }
        }
    }

    // ��ȡ�Ѷ�Ԫ��
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

    // ά���ѣ�����ʱ��moment������ͳ�Ʊ�Ե�ڵ�����
    void maintainHeapsOfMoment(int moment)
    {
        // 1. ����ÿ����Ե�ڵ㣬���������˱仯��ʱ�̣�Ҫ���¶�
        for (int s = 0; s < numSites; ++s)
        {
            // �ĸ������Ԫ�ظ����ˣ���ѹ���ĸ��ѣ�����ԭ������ͬԪ�ؾ�������
            if (siteInWhichHeap[s][moment] == 95)
                heap95[s].push(PII(siteFlow[moment][s], moment));
            else
                heap5[s].push(PII(siteFlow[moment][s], moment));
        }
        // 2. ����ÿ����Ե�ڵ㣬����������֮���ƽ��״̬��ʹ��С�Ѷ�<=��Ѷ�
        for (int siteId = 0; siteId < numSites; ++siteId)
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
        // 3. ���ù���ӷ�׵Ķѣ���ֹ�˷��ڴ�
        for (int s = 0; s < numSites; ++s)
            if (heap95[s].size() + heap5[s].size() > 100000)
                resetHeap(s);
    }

    // ���㵱ǰ״̬���ܳɱ�
    int calculateCost()
    {
        int cost = 0;
        for (int s = 0; s < numSites; ++s)
            cost += getHeapTop(heap95, s).first;
        return cost;
    }

    // ����ʱ�̣����·������
    void reassignFlowOfAMoment(int moment)
    {
        // ͳ��ÿ���ڵ���Ż���ֵ
        vector<double> siteCost(numSites);
        double sumOptimScore = 0;
        for (int s = 0; s < numSites; ++s)
        {
            double optimValue = getHeapTop(heap5, s).first - getHeapTop(heap95, s).first; // �Ż���
            siteCost[s] = optimValue;
            sumOptimScore += siteCost[s];
        }
        // ������ۣ���Χ[0,2]��ԽСԽ���ȿ���
        for (int s = 0; s < numSites; ++s)
        {
            siteCost[s] /= sumOptimScore;
            if (siteInWhichHeap[moment][s] == 95)
                siteCost[s] += 1;
            else
                siteCost[s] = 1 - siteCost[s];
        }

        // to �����۷�������

        // ͳ�ƽڵ�ĳ�������
        vector<PII> sites(numSites, PII(0, 0)); //<��������,���>
        for (int s = 0; s < numSites; ++s)
        {
            sites[s].second = s;
            for (int u = 0; u < numUsers; ++u)
            {
                if (qos.get(s, u) < qosConstraint)
                {
                    sites[s].first = min(siteList.getBandwidth(s), sites[s].first + demand.getDemand(moment, u));
                }
            }
        }
        // ��ʼ����
        vector<int> userDemand(numUsers);
        for (int u = 0; u < numUsers; ++u)
            userDemand[u] = demand.getDemand(moment, u);
    }

public:
    int run(int epochs = 10)
    {
        // 1. ��ʼ��һ�����н�
        for (int t = 0; t < numMoments; ++t)
        {
            // ��ǰʱ�̵��û�����
            vector<int> userDemand(numUsers); // ��ǰʱ�̱�Ե�ڵ�sʣ������
            for (int u = 0; u < numUsers; ++u)
                userDemand[u] = demand.getDemand(t, u);
            // ÿ����Ե�ڵ��ռ�����
            for (int siteId = 0; siteId < numSites; ++siteId)
            {
                int capacity = siteList.getBandwidth(siteId);
                for (int u = 0; u < numUsers; ++u)
                {
                    if (qos.get(siteId, u) >= qosConstraint)
                        continue;
                    int bandwidth = min(userDemand[u], capacity);
                    add(t, siteId, u, bandwidth);
                    capacity -= bandwidth;
                    userDemand[u] -= bandwidth;
                }
            }
        }

        // 2. ��ʼ����
        for (int s = 0; s < numSites; ++s)
            resetHeap(s);

        // 3. ������Ե�ڵ㣬�滮����
        for (int epoch = 0; epoch < epochs; ++epoch)
        {
            for (int siteId = 0; siteId < numSites; ++siteId)
            {
                PII state = getHeapTop(heap95, siteId, true); // ȡ��95�ٷ�λ��Ӧʱ��
                // int currentSiteFlow = state.first;            // 95�ٷ�λʱ�̶�Ӧ�ı�Ե�ڵ�ʵ������
                int moment = state.second; // 95�ٷ�λ������ʱ��

                // todo
                reassignFlowOfAMoment(moment);

                // ��ǰʱ�����������仯����Ҫά����
                maintainHeapsOfMoment(moment);
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
