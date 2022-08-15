#include <bits/stdc++.h>
using namespace std;

//去除字符串首尾空白符
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

//边缘节点列表
class SiteList
{
private:
    vector<string> siteNames;
    map<string, int> siteId;
    vector<int> siteBandwidths;

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
        getline(fin, line); //表头
        while (getline(fin, line))
        {
            istringstream sin(strip(line));
            getline(sin, siteName, ',');  //读取节点名
            getline(sin, bandwidth, ','); //读取带宽限制
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

//用户需求表
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
        //处理表头
        getline(fin, line);
        istringstream sin(strip(line));
        getline(sin, username, ','); //第一个不是用户名
        while (getline(sin, username, ','))
        {
            userNames.push_back(strip(username));
        }
        for (int i = 0; i < (int)userNames.size(); ++i)
            userId[userNames[i]] = i;
        // 处理需求值
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

//时延表
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
        // 已读取文件，初始化qos二维数组大小
        qos.resize(siteList.getSiteNames().size());
        for (auto &eu : qos)
            eu.resize(demand.getUserNames().size());

        //读取数据
        string line, siteName, username, q;
        //读取第一行，用户名
        getline(fin, line);
        istringstream sin(strip(line));
        getline(sin, username, ',');                          //第一个不是用户名
        vector<int> userRealId(demand.getUserNames().size()); //映射为真实用户id
        for (int u = 0; getline(sin, username, ','); ++u)
        {
            userRealId[u] = demand.getUserId(username); //获取用户的真实id
        }
        // 读取时延矩阵qos
        for (int t = 0; getline(fin, line); t++) // t代表时刻
        {
            istringstream sin(strip(line));
            getline(sin, siteName, ',');
            int siteId = siteList.getSiteId(siteName); //获取边缘节点的真实id
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

//方案表
class Solution
{
private:
    //方案表格scheme
    // 1-dim: 时刻
    // 2-dim: 用户
    // 3-dim: 当前用户的流量列表
    // 4-dim: pair<边缘节点名，流量>
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

        //遍历方案进行输出
        for (int t = 0; t < (int)scheme.size(); ++t) //遍历时间
        {
            //对于每一个时刻，遍历用户
            // cout << "The moment " << t << " " << moments[t] << " is writing" << endl;
            for (int u = 0; u < (int)scheme[t].size(); ++u)
            {
                //对于t时刻的用户u，输出其连接的所有边缘节点
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

// 读取config.ini配置文件
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
            if (eq_pos != -1 && line.substr(0, eq_pos) == keyName) //找到配置项，返回值
                return line.substr(eq_pos + 1);
        }
    }
    return ret;
}

//解决方案
void solve(string dataDir, string outputPath)
{
    /****** 读取数据 ******/
    int qos_constraint = atoi(getConfigIni(dataDir + "/config.ini", "config", "qos_constraint").c_str());
    SiteList siteList(dataDir + "/site_bandwidth.csv");
    Demand demand(dataDir + "/demand.csv");
    Qos qos(dataDir + "/qos.csv", siteList, demand);
    Solution solution(demand.getMoments(), siteList.getSiteNames(), demand.getUserNames());

    /***** 此处写策略 *****/
    for (int t = 0; t < (int)demand.getMoments().size(); ++t)
    {
        auto band_capacity = siteList.getSiteBandwidth(); //当前时刻t每个边缘节点剩余容量（深拷贝）
        for (int u = 0; u < (int)demand.getUserNames().size(); ++u)
        {
            int cur_demand = demand.getDemand(t, u);                              //时刻t用户u的需求量
            for (int s = 0; s < (int)band_capacity.size() && cur_demand > 0; ++s) //遍历边缘节点
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

    /***** 将结果写入文件 *****/
    solution.output(outputPath);
}

int main()
{
    if (ifstream("/data/config.ini").good()) //线上环境
        solve("/data", "/output/solution.txt");
    else if (ifstream("../data/config.ini").good()) // windows测试环境
        solve("../data", "../data/solution.txt");
    else // linux测试环境
        solve("../CodeCraft-2022/data", "../CodeCraft-2022/data/solution.txt");
    return 0;
}
