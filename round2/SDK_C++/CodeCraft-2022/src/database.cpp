#include "database.h"

// ���캯��
Database::Database(const string &data_dir)
{
    string config_path = data_dir + "/config.ini";
    string demand_path = data_dir + "/demand.csv";
    string bandwidth_path = data_dir + "/site_bandwidth.csv";
    string qos_path = data_dir + "/qos.csv";
    this->qos_constraint = atoi(read_config(config_path, "config", "qos_constraint").c_str());
    this->base_cost = atoi(read_config(config_path, "config", "base_cost").c_str());
    process_demand(demand_path);
    process_bandwidth(bandwidth_path);
    process_qos(qos_path);
}

/************************************ ��ȡ���ݼ� *****************************************/
void Database::process_demand(const string &path)
{
    auto demand_csv = read_csv(path);
    // ��ȡ�û���
    for (size_t i = 2; i < demand_csv[0].size(); i++)
        this->usernames.push_back(demand_csv[0][i]);
    // ��ȡ����
    for (size_t i = 1; i < demand_csv.size(); i++)
    {
        const auto &line = demand_csv[i];
        if (this->moments.size() == 0 || this->moments.back() != line[0]) // ��ʱ��
        {
            this->moments.push_back(line[0]);                   // ���浱ǰʱ��
            this->stream_names.emplace_back();                  // ���赱ǰʱ��
            this->demand.emplace_back();                        // ���赱ǰʱ��
            this->demand.back().resize(this->usernames.size()); // ���õ�ǰʱ�̵��û���
        }
        this->stream_names.back().push_back(line[1]); // ��ǰʱ������һ����
        for (size_t j = 2; j < line.size(); j++)
        {
            this->demand.back()[j - 2].push_back(atoi(line[j].c_str())); // ��ǰʱ���û�(j-2)����һ����
        }
    }
}

void Database::process_bandwidth(const string &path)
{
    auto bandwidth_csv = read_csv(path);
    for (size_t i = 1; i < bandwidth_csv.size(); i++)
    {
        this->sitenames.push_back(bandwidth_csv[i][0]);
        this->bandwidth.push_back(atoi(bandwidth_csv[i][1].c_str()));
    }
}

void Database::process_qos(const string &path)
{
    auto qos_csv = read_csv(path);
    unordered_map<string, int> user2index, site2index;
    for (size_t i = 0; i < usernames.size(); i++)
        user2index[usernames[i]] = i;
    for (size_t i = 0; i < sitenames.size(); i++)
        site2index[sitenames[i]] = i;

    this->qos.resize(qos_csv.size() - 1, vector<int>(qos_csv[0].size() - 1, 0));
    for (size_t i = 1; i < qos_csv.size(); i++)
    {
        for (size_t j = 1; j < qos_csv[i].size(); j++)
            this->qos[site2index[qos_csv[i][0]]][user2index[qos_csv[0][j]]] = atoi(qos_csv[i][j].c_str());
    }
}

void Database::add_flow_plan(int moment, int user, int site, int stream, int flow, bool reset)
{
    if (reset)
        flow_plan[{moment, user, site, stream}] = 0;
    flow_plan[{moment, user, site, stream}] += flow;
}
void Database::add_free10_site(int site_id, bool erase)
{
    if (erase)
        this->free10sites.erase(site_id);
    this->free10sites.insert(site_id);
}

/************************************ �����������ָ���ļ� ********************************/
void Database::output_to_file(const string &path)
{
    cout << " -- Output solution to file " << path << endl;
    ofstream fout(path, ios::out);
    int first = 0;
    for (auto site : this->free10sites)
        fout << (first++ ? "," : "") << this->sitenames[site];
    fout << endl;
    for (int t = 0; t < num_moments(); t++)
    {
        for (int u = 0; u < num_users(); u++)
        {
            fout << usernames[u] << ":";
            bool first_site = true;
            for (int s = 0; s < num_sites(); s++)
            {
                auto stream_begin = flow_plan.lower_bound({t, u, s, 0});
                auto stream_end = flow_plan.upper_bound({t, u, s, num_streams(t)});
                bool has_flow = false;
                for (auto it = stream_begin; it != stream_end; it++)
                    if (flow_plan[{t, u, s, it->first.back()}])
                        has_flow = true;
                if (!has_flow) // ��ǰu->s������,�������
                    continue;
                // ȷ���������������ʼ���=========================================
                if (!first_site)
                    fout << ",";
                first_site = false;
                fout << "<" << sitenames[s];
                for (auto it = stream_begin; it != stream_end; it++)
                    if (flow_plan[{t, u, s, it->first.back()}])
                        fout << "," << stream_names[t][it->first.back()];
                fout << ">";
            }
            fout << endl;
        }
    }
    fout.close();
}

/************************************* ��̬���� *****************************************/
// ��ȡconfig�ļ�
string Database::read_config(const string &config_path, const string &app_name, const string &key_name)
{
    ifstream fin(config_path);
    if (!fin)
    {
        cout << "No such a file " << config_path << endl;
        exit(-101);
    }
    string line, curAppName = string(), retValue = string();
    while (getline(fin, line))
    {
        line = strip(line);
        if (line.length() > 2 && line[0] == '[' && line.back() == ']')
            curAppName = line.substr(1, line.length() - 2);
        if (curAppName == app_name)
        {
            int eq_pos = line.find('=');
            if (eq_pos != -1 && strip(line.substr(0, eq_pos)) == key_name) //�ҵ�������
            {
                retValue = strip(line.substr(eq_pos + 1));
                break;
            }
        }
    }
    return retValue;
}

// ��ȡcsv�ļ�
vector<vector<string>> Database::read_csv(const string &csv_path)
{
    vector<vector<string>> matrix;
    ifstream fin(csv_path, ios::in);
    if (!fin)
    {
        cout << "Can't open file " << csv_path << endl;
        exit(-102);
    }
    string line, element;
    while (getline(fin, line))
    {
        matrix.emplace_back();
        istringstream sin(strip(line));
        while (getline(sin, element, ','))
            matrix.back().push_back(strip(element));
    }
    fin.close();
    return matrix;
}

// ȥ���ַ�����β�հ׷�
string Database::strip(const string &str)
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
