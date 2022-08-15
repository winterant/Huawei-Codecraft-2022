#include <bits/stdc++.h>
using namespace std;

//ȥ���ַ�����β�հ׷�
string strip(string &s)
{
    auto isw = [](char ch)
    {
        return ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t';
    };
    int i = 0, j = s.length() - 1;
    while (i <= j && isw(s[i]))
        ++i;
    while (i <= j && isw(s[j]))
        --j;
    if (i <= j)
        return s.substr(i, j - i + 1);
    return string();
}

//��Ե�ڵ��б�
class SiteList
{
private:
    vector<string> siteNames;
    map<string, int> siteId;
    vector<int> siteBandwidths;

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
        getline(fin, line); //��ͷ
        while (getline(fin, line))
        {
            istringstream sin(strip(line));
            getline(sin, siteName, ',');  //��ȡ�ڵ���
            getline(sin, bandwidth, ','); //��ȡ��������
            siteNames.push_back(siteName);
            siteBandwidths.push_back(atoi(bandwidth.c_str()));
        }
        fin.close();
        for (int i = 0; i < (int)siteNames.size(); ++i)
            siteId[siteNames[i]] = i;
    }

    vector<string> &getSiteNames()
    {
        return siteNames;
    }
    vector<int> &getSiteBandwidth()
    {
        return siteBandwidths;
    }
    int getSiteId(string &siteName)
    {
        return siteId[siteName];
    }
};

//�û������
class Demand
{
private:
    vector<string> moments;
    vector<string> userNames;
    map<string, int> userId;
    vector<vector<int>> demand;

public:
    Demand(string filepath)
    {
        ifstream fin(filepath, ios::in);
        if (!fin)
        {
            cout << "Can't open file " << filepath << endl;
            exit(3);
        }
        string line, username, moment, bandwidth;
        //�����ͷ
        getline(fin, line);
        istringstream sin(strip(line));
        getline(sin, username, ','); //��һ�������û���
        while (getline(sin, username, ','))
        {
            userNames.push_back(strip(username));
        }
        for (int i = 0; i < (int)userNames.size(); ++i)
            userId[userNames[i]] = i;
        // ��������ֵ
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
    vector<string> &getMoments()
    {
        return moments;
    }
    vector<string> &getUserNames()
    {
        return userNames;
    }
    int getUserId(string &userName)
    {
        return userId[userName];
    }
};

//ʱ�ӱ�
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
        // �Ѷ�ȡ�ļ�����ʼ��qos��ά�����С
        qos.resize(siteList.getSiteNames().size());
        for (auto &eu : qos)
            eu.resize(demand.getUserNames().size());

        //��ȡ����
        string line, siteName, username, q;
        //��ȡ��һ�У��û���
        getline(fin, line);
        istringstream sin(strip(line));
        getline(sin, username, ',');                          //��һ�������û���
        vector<int> userRealId(demand.getUserNames().size()); //ӳ��Ϊ��ʵ�û�id
        for (int u = 0; getline(sin, username, ','); ++u)
        {
            userRealId[u] = demand.getUserId(username); //��ȡ�û�����ʵid
        }
        // ��ȡʱ�Ӿ���qos
        for (int t = 0; getline(fin, line); t++) // t����ʱ��
        {
            istringstream sin(strip(line));
            getline(sin, siteName, ',');
            int siteId = siteList.getSiteId(siteName); //��ȡ��Ե�ڵ����ʵid
            for (int u = 0; getline(sin, q, ','); ++u)
            {
                qos[siteId][userRealId[u]] = atoi(strip(q).c_str());
            }
        }
        fin.close();
    }
    int get(int siteId, int userId)
    {
        return qos[siteId][userId];
    }
};

//������
class Solution
{
private:
    //�������scheme
    // 1-dim: ʱ��
    // 2-dim: �û�
    // 3-dim: ��ǰ�û��������б�
    // 4-dim: pair<��Ե�ڵ���������>
    vector<vector<vector<pair<string, int>>>> scheme;
    vector<string> &moments;
    vector<string> &siteNames;
    vector<string> &userNames;

public:
    Solution(vector<string> &moments, vector<string> &siteNames, vector<string> &userNames)
        : moments(moments), siteNames(siteNames), userNames(userNames)
    {
        scheme.resize(moments.size());
        for (auto &i : scheme)
            i.resize(userNames.size());
    }

    void add(int moment, int userId, int siteId, int bandwidth)
    {
        scheme[moment][userId].push_back(pair<string, int>(siteNames[siteId], bandwidth));
    }

    void output(string filepath)
    {
        cout << "------ Start to output solution to file --------" << endl;
        cout << "Solution contains " << moments.size() << " moments and " << userNames.size() << " users." << endl;
        ofstream fout(filepath, ios::out);

        //���������������
        for (int t = 0; t < (int)scheme.size(); ++t) //����ʱ��
        {
            //����ÿһ��ʱ�̣������û�
            // cout << "The moment " << t << " " << moments[t] << " is writing" << endl;
            for (int u = 0; u < (int)scheme[t].size(); ++u)
            {
                //����tʱ�̵��û�u����������ӵ����б�Ե�ڵ�
                fout << userNames[u] << ":";
                bool first = true;
                for (auto &pr : scheme[t][u])
                {
                    if (!first)
                        fout << ',';
                    first = false;
                    fout << '<' << pr.first << ',' << pr.second << '>';
                }
                fout << endl;
            }
        }
        fout.close();
        cout << "Wrote solution to file " << filepath << endl;
    }
};

// ��ȡconfig.ini�����ļ�
string getConfigIni(string filepath, string appName, string keyName, string defaultValue = string())
{
    ifstream fin(filepath);
    if (!fin)
    {
        cout << "No such a file " << filepath << endl;
        exit(1);
    }
    string curAppName = string(), line, ret = defaultValue;
    while (getline(fin, line))
    {
        line = strip(line);
        if (line.length() > 2 && line[0] == '[' && line.back() == ']')
        {
            curAppName = line.substr(1, line.length() - 2);
        }
        if (curAppName == appName)
        {
            int eq_pos = line.find('=');
            if (eq_pos != -1 && line.substr(0, eq_pos) == keyName) //�ҵ����������ֵ
                return line.substr(eq_pos + 1);
        }
    }
    return ret;
}

//�������
void solve(string dataDir, string outputPath)
{
    /****** ��ȡ���� ******/
    int qos_constraint = atoi(getConfigIni(dataDir + "/config.ini", "config", "qos_constraint").c_str());
    SiteList siteList(dataDir + "/site_bandwidth.csv");
    Demand demand(dataDir + "/demand.csv");
    Qos qos(dataDir + "/qos.csv", siteList, demand);
    Solution solution(demand.getMoments(), siteList.getSiteNames(), demand.getUserNames());

    /***** �˴�д���� *****/
    for (int t = 0; t < (int)demand.getMoments().size(); ++t)
    {
        auto band_capacity = siteList.getSiteBandwidth(); //��ǰʱ��tÿ����Ե�ڵ�ʣ�������������
        for (int u = 0; u < (int)demand.getUserNames().size(); ++u)
        {
            int cur_demand = demand.getDemand(t, u);                              //ʱ��t�û�u��������
            for (int s = 0; s < (int)band_capacity.size() && cur_demand > 0; ++s) //������Ե�ڵ�
            {
                if (band_capacity[s] == 0 || qos.get(s, u) >= qos_constraint)
                    continue;
                int bandwidth = min(cur_demand, band_capacity[s]);
                solution.add(t, u, s, bandwidth);
                cur_demand -= bandwidth;
                band_capacity[s] -= bandwidth;
            }
        }
    }

    /***** �����д���ļ� *****/
    solution.output(outputPath);
}

int main()
{
    if (ifstream("/data/config.ini").good()) //���ϻ���
        solve("/data", "/output/solution.txt");
    else if (ifstream("../data/config.ini").good()) // windows���Ի���
        solve("../data", "../data/solution.txt");
    else // linux���Ի���
        solve("../CodeCraft-2022/data", "../CodeCraft-2022/data/solution.txt");
    return 0;
}
