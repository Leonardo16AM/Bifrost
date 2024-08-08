#ifndef INERTIAL_FLOW_H
#define INERTIAL_FLOW_H

#include "dinic.cpp"
#include "graph.h"
#include <numeric> 
#include <unordered_set>
#include <random>
#include <map>
#include <algorithm>
#include <vector>


std::vector<Edge> latitudinal_mincut(Graph& graph,int n);
Graph extract_subgraph(const Graph& original_graph, const std::unordered_set<int>& node_ids);
struct pair_hash;
void partition_graph_recursively(const Graph& graph, std::vector<std::pair<int, int>>& node_partition, int& current_partition_id, const std::unordered_set<int>& subset);
std::vector<std::pair<int, int>> inertial_flow_partition(const Graph& graph);

#endif // INERTIAL_FLOW_H