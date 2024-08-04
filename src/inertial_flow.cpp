#include "inertial_flow.h"

std::vector<Edge> latitudinal_mincut(Graph& graph,int n) {
    std::vector<Node> sorted_nodes = graph.nodes; // Copia de los nodos
    std::sort(sorted_nodes.begin(), sorted_nodes.end(), [](const Node& a, const Node& b) {
        return a.lat < b.lat; 
    });

    int k = sorted_nodes.size() / 4; // 25% de los nodos

    dinic<int> flow(n + 2); // Incluye nodos extra para source y sink
    int source = n, sink = n + 1;

    for (int i = 0; i < k; ++i) {
       flow.add_edge(source, sorted_nodes[i].id, 10000);
        flow.add_edge(sorted_nodes[sorted_nodes.size() - i - 1].id, sink, 10000);
    }
    for (auto& e : graph.edges) {
        flow.add_edge(e.source, e.target, 1); // Usar capacidad 1 para simplificar
    }
    flow.max_flow(source, sink); // Calcular el flujo máximo
    auto mincut_edges_dinic = flow.get_mincut(source, sink); // Obtener el mincut
    std::vector<Edge> result;
    for (const auto& e : mincut_edges_dinic) {
        Edge converted_edge = {e.src, e.dst, "", "", false, "", "", "", "", false, "", "", "", "", "", "", ""};
        result.push_back(converted_edge);
    }
    return result;
}



Graph extract_subgraph(const Graph& original_graph, const std::unordered_set<int>& node_ids) {
    std::vector<Node> subgraph_nodes;
    std::vector<Edge> subgraph_edges;

    for (const auto& node : original_graph.nodes) 
        if (node_ids.find(node.id) != node_ids.end()) 
            subgraph_nodes.push_back(node);
    
    for (const auto& edge : original_graph.edges) 
        if (node_ids.find(edge.source) != node_ids.end() && node_ids.find(edge.target) != node_ids.end()) 
            subgraph_edges.push_back(edge);
     
    return Graph(subgraph_nodes, subgraph_edges);
}

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &pair) const {
        auto hash1 = std::hash<T1>{}(pair.first);
        auto hash2 = std::hash<T2>{}(pair.second);
        return hash1 ^ hash2;
    }
};



void partition_graph_recursively(const Graph& graph, std::vector<std::pair<int, int>>& node_partition, int& current_partition_id, const std::unordered_set<int>& subset) {
    if (subset.size() < 400) {
        for (int node_id : subset) {
            node_partition.emplace_back(node_id, current_partition_id);
        }
        current_partition_id++;
        return;
    }
    
    Graph subgraph = extract_subgraph(graph, subset);
     
    auto mincut_edges = latitudinal_mincut(subgraph,graph.nodes.size());
     
    std::unordered_set<int> set_S = subset, set_T;
    std::unordered_set<std::pair<int, int>, pair_hash> cut_edges;

    for (const auto& edge : mincut_edges) {
        cut_edges.insert({edge.source, edge.target});
        cut_edges.insert({edge.target, edge.source});
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, subset.size() - 1);
    auto it = subset.begin();
    std::advance(it, distrib(gen));
    int start_node = *it;

    std::queue<int> queue;
    std::unordered_set<int> visited;
    queue.push(start_node);
    visited.insert(start_node);


    while (!queue.empty()) {
        int current = queue.front();
        queue.pop();

        auto it = graph.adj_list.find(current);
        if (it != graph.adj_list.end()) {
            for (const auto& adj : it->second) {
                if (subset.count(adj.first) && visited.count(adj.first) == 0 && cut_edges.count({current, adj.first}) == 0) {
                    queue.push(adj.first);
                    visited.insert(adj.first);
                }
            }
        }
    }
    
    // Dividir en S y T basados en los visitados por el BFS
    set_S.clear();
    for (int node : subset) {
        if (visited.count(node)) {
            set_S.insert(node);
        } else {
            set_T.insert(node);
        }
    }

    partition_graph_recursively(graph, node_partition, current_partition_id, set_S);
    partition_graph_recursively(graph, node_partition, current_partition_id, set_T);
}




std::vector<std::pair<int, int>> inertial_flow_partition(const Graph& graph) {
    std::vector<std::pair<int, int>> node_partition;
    int current_partition_id = 0;
    std::unordered_set<int> initial_set;
    for (const auto& node : graph.nodes) {
        initial_set.insert(node.id);
    }
    partition_graph_recursively(graph, node_partition, current_partition_id, initial_set);
    return node_partition;
}

