#ifndef SOLUTION_H
#define SOLUTION_H

#include <bits/stdc++.h>
#include "database.h"
#include "graph.h"
using namespace std;

class Solution
{
public:
    Solution(Database &database) : db(database) {}
    void run(const string &output_path);

private:
    Database &db;
};

#endif // SOLUTION_H
