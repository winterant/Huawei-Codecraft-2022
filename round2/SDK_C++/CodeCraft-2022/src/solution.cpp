#include "solution.h"

void Solution::run(const string &output_path = string())
{
    /***************************** 解决方案 ******************************/
    // user的度
    vector<int> user_degree(db.num_users(), 0);
    for (int u = 0; u < db.num_users(); u++)
        for (int s = 0; s < db.num_sites(); s++)
            if (db.is_qos_ok(s, u))
                user_degree[u]++;
    // sorted user id
    vector<int> sorted_user(db.num_users());
    sort(sorted_user.begin(), sorted_user.end(), [=](int i, int j)
         { return user_degree[i] < user_degree[j]; });
    // 计算user可连接容量总量
    vector<vector<int64_t>> user_conn_cap(db.num_moments(), vector<int64_t>(db.num_users(), 0));
    for (int t = 0; t < db.num_moments(); t++)
        for (int u = 0; u < db.num_users(); u++)
            for (int s = 0; s < db.num_sites(); s++)
                if (db.is_qos_ok(s, u))
                    user_conn_cap[t][u] += db.get_bandwidth(s);
    // 计算潜力值
    vector<vector<int64_t>> potential(db.num_sites(), vector<int64_t>(db.num_moments(), 0));
    for (int s = 0; s < db.num_sites(); s++)
    {
        for (int t = 0; t < db.num_moments(); t++)
            for (int u = 0; u < db.num_users(); u++)
                for (int stream_id = 0; stream_id < db.num_streams(t); stream_id++)
                    potential[s][t] += db.get_demand(t, u, stream_id);
    }
    // 1. 选取免费节点
    vector<vector<bool>> is_selected_node(db.num_sites(), vector<bool>(db.num_moments(), false));
    vector<int> num_selected_node(db.num_sites()); // 每个节点选中的个数
    set<int> selected10;                           // 被选为10%免费点的边缘节点
    while (false)
    {
        int64_t best_val = -1;
        pair<int, int> best_node = {-1, -1}; // <s,t>
        for (int s = 0; s < db.num_sites(); s++)
        {
            if (num_selected_node[s] == db.num_free10_moments())
                continue;
            else if (num_selected_node[s] == db.num_free5_moments() && selected10.size() == 10)
                continue;

            for (int t = 0; t < db.num_moments(); t++)
            {
                if (is_selected_node[s][t])
                    continue;
                int64_t val = potential[s][t]; // 潜力值
                if (best_val < val)
                {
                    best_val = val;
                    best_node = {s, t};
                }
            }
        }
        if (best_val == -1)
            break;
        int s = best_node.first, t = best_node.second;
        is_selected_node[s][t] = true;
        // todo: update potential
        if (++num_selected_node[s] > db.num_free5_moments())
            selected10.insert(s);
    }

    // for (int i = 0; i < db.num_sites(); i++)
    //     cout << "site " << i << "   number: " << num_selected_node[i] << endl;

    // 记录free10
    // for (int s : selected10)
    //     db.add_free10_site(s);

    // 2. 平均分配
    vector<vector<int>> sites_flow(db.num_moments(), vector<int>(db.num_sites(), 0));
    for (int t = 0; t < db.num_moments(); t++)
    {
        for (int u = 0; u < db.num_users(); u++)
        // for (int u : sorted_user)
        {
            for (int sm = 0; sm < db.num_streams(t); sm++)
            {
                if (db.get_demand(t, u, sm) == 0)
                    continue;
                // selected node first.
                // biggest rest bandwidth first.
                int biggest_band = -1;
                int best_site = -1;
                for (int s = 0; s < db.num_sites(); s++)
                {
                    int remain_band = db.get_bandwidth(s) - sites_flow[t][s];
                    if (!db.is_qos_ok(s, u) || remain_band < db.get_demand(t, u, sm))
                        continue;
                    if (biggest_band < remain_band)
                    {
                        biggest_band = remain_band;
                        best_site = s;
                    }
                }
                if (best_site != -1)
                {
                    db.add_flow_plan(t, u, best_site, sm, db.get_demand(t, u, sm));
                    sites_flow[t][best_site] += db.get_demand(t, u, sm);
                }
            }
        }
    }
    // 3. 调整

    db.clear_free10_site();
    for (int i = db.num_sites() - 1 - 10, j = 0; j < 10; j++, i++)
        db.add_free10_site(i);
    // vector<int>cost90(db.num_sites(),0);
    // for(int s=0;s<db.num_sites();s++)
    // {

    // }
    /********************************************************************/
    if (output_path.length())
        db.output_to_file(output_path);
}