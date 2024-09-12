#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <queue>
#include <stack>
#include <limits>
#include <filesystem> // C++17 or later
#include <cmath>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <utility>
#include <iomanip>

struct Node {
    int id;
    float lat=0.0;
    float lon=0.0;
    int street_count=0;
    std::string highway="";
    std::string geometry="";
    double betweenness_centrality = 0.0; // Añadir campo para la centralidad de intermediación
};

struct Edge {
    int source;
    int target;
    std::string key; //int
    std::string osmid;
    bool oneway;
    std::string lanes; //int
    std::string name;
    std::string highway;
    std::string maxspeed; //double
    bool reversed;
    double length; //double
    std::string geometry;
    std::string bridge;
    std::string ref;
    std::string junction;
    std::string tunnel;
    std::string width; //double
    std::string access;
    std::string service;
};

class Graph {
public:
    Graph(const std::vector<Node>& nodes_, const std::vector<Edge>& edges_);
    Graph();
    const std::vector<std::pair<int, int>>& operator[](int node_id) const;
    void calculate_betweenness_centrality(); // Añadir declaración de función
    void save_betweenness_to_csv(const std::string& filename) const; // Declaración para guardar en CSV
    void load_betweenness_from_csv(const std::string& filename); // Declaración para cargar desde CSV
    std::vector<int> a_star(int start_id, int goal_id) const; // Añadir declaración de A*
    std::unordered_map<int, std::pair<int, double>> dijkstra(int start_id, const std::unordered_set<int>& visitable_nodes) const;
    std::vector<Node> nodes;
    std::vector<Edge> edges;
    std::unordered_map<int, std::vector<std::pair<int, int>>> adj_list;
    std::vector<int> reconstruct_path(int start_id, int goal_id, const std::unordered_map<int, std::pair<int, double>>& dijkstra_result) const;
    Graph to_bidirectional() const;

private:
    double heuristic(int node_id1, int node_id2) const;
    double get_node_distance(int node_id1, int node_id2) const;
    std::unordered_map<int, Node> node_map;
};

bool file_exists_and_readable(const std::string& filename);
std::vector<Node> read_nodes(const std::string& filename, std::unordered_map<long long, int>& node_map);
std::vector<Edge> read_edges(const std::string& filename, const std::unordered_map<long long, int>& node_map);
Graph build_map(const std::string& nodesfile, const std::string& edgesfile);
#endif // GRAPH_H
