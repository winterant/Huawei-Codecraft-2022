#include "graph.h"

Graph::Graph(int num_node)
{
    graph.resize(num_node);
}

// ���ڽӱ������from->to�����
void Graph::add_edge(int from, int to, int64_t cost, int64_t cap)
{
    graph[from].push_back(Edge(from, to, cost, cap, 0, true));
    graph[to].push_back(Edge(to, from, -cost, 0, 0, false)); //���ڱ�
    graph[from].back().reversed_edge = pair<int, int>(to, graph[to].size() - 1);
    graph[to].back().reversed_edge = pair<int, int>(from, graph[from].size() - 1);
}
