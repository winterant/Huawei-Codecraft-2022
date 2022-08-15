#include <bits/stdc++.h>
#include "database.h"
#include "solution.h"
using namespace std;

int main()
{
    /****************************** ȷ�����ݼ�·�� ********************************/
    string data_dir = "/data", output_dir = "/output"; // Ĭ������·��
    string debug_dir = "../dataset/data";              // ���µ���·��
    if (ifstream(debug_dir + "/config.ini").good())    // cmake��������·��
    {
        data_dir = output_dir = debug_dir;
    }
    else if (ifstream("../" + debug_dir + "/config.ini").good()) // windows��������·��
    {
        data_dir = output_dir = "../" + debug_dir;
    }

    /******************************* �������  ******************************/
    Database database(data_dir);
    Solution solution(database);
    solution.run(output_dir + "/solution.txt");
    return 0;
}
