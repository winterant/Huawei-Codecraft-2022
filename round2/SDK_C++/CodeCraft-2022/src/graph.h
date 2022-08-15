#ifndef GRAPH_H
#define GRAPH_H

#include <bits/stdc++.h>
using namespace std;

class Graph
{
private:
    struct Edge
    {
        int from, to;
        int64_t cost;
        int64_t cap, flow;
        bool isForward;
        pair<int, int> reversed_edge; // <node id, edge index>
        Edge(int from, int to, int64_t cost, int64_t cap, int64_t flow, bool isForward)
            : from(from), to(to), cost(cost), cap(cap), flow(flow), isForward(isForward) {}
    };
    vector<vector<Edge>> graph;

public:
    explicit Graph(int num_node);
    int num_nodes() { return graph.size(); }
    void add_edge(int from, int to, int64_t cost, int64_t cap); //添加from->to有向边
    

private:
    Edge &reversed_edge(const Edge &edge) { return graph[edge.reversed_edge.first][edge.reversed_edge.second]; } // 获取反向边
};

#endif // GRAPH_H
