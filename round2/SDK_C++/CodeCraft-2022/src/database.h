#ifndef DATABASE_H
#define DATABASE_H

#include <bits/stdc++.h>
using namespace std;

class Database
{
public:
    explicit Database(const string &data_dir);
    void output_to_file(const string &path);

    int get_bandwidth(int site_id) { return bandwidth[site_id]; }
    int get_demand(int moment, int user_id, int stream_id) { return demand[moment][user_id][stream_id]; }
    bool is_qos_ok(int site_id, int user_id) { return qos[site_id][user_id] < qos_constraint; }
    int num_moments() { return moments.size(); }
    int num_free10_moments() { return moments.size() / 10; }
    int num_free5_moments() { return moments.size() / 20; }
    int num_sites() { return sitenames.size(); }
    int num_users() { return usernames.size(); }
    int num_streams(int moment) { return stream_names[moment].size(); }

    void add_flow_plan(int moment, int user, int site, int stream, int flow, bool reset = false);
    void add_free10_site(int, bool = false);
    void clear_free10_site(){free10sites.clear();}

private:
    void process_demand(const string &path);
    void process_bandwidth(const string &path);
    void process_qos(const string &path);

    static string read_config(const string &config_path, const string &app_name, const string &key_name);
    static vector<vector<string>> read_csv(const string &csv_path);
    static string strip(const string &s);

private:
    int qos_constraint; // 时延上限
    int base_cost;      // 有流量时，费用下限

    vector<string> usernames;            // 用户名列表
    vector<string> moments;              // [moment] 时间列表
    vector<vector<string>> stream_names; // [moment][k] 时刻moment的第k种流的名字
    vector<vector<vector<int>>> demand;  // [moment][user][stream] 需求

    vector<string> sitenames; // 节点名列表
    vector<int> bandwidth;    // [site] 节点容量限制

    vector<vector<int>> qos; // [site][user] 时延

    map<array<int, 4>, int> flow_plan; // [moment][user][site][stream]=流量
    set<int> free10sites;              // 选取的10个10%免费节点
};

#endif // DATABASE_H
