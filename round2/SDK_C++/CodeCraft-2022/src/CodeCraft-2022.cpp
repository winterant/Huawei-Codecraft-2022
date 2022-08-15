#include <bits/stdc++.h>
#include "database.h"
#include "solution.h"
using namespace std;

int main()
{
    /****************************** 确定数据集路径 ********************************/
    string data_dir = "/data", output_dir = "/output"; // 默认线上路径
    string debug_dir = "../dataset/data";              // 线下调试路径
    if (ifstream(debug_dir + "/config.ini").good())    // cmake调试数据路径
    {
        data_dir = output_dir = debug_dir;
    }
    else if (ifstream("../" + debug_dir + "/config.ini").good()) // windows调试数据路径
    {
        data_dir = output_dir = "../" + debug_dir;
    }

    /******************************* 解决方案  ******************************/
    Database database(data_dir);
    Solution solution(database);
    solution.run(output_dir + "/solution.txt");
    return 0;
}
