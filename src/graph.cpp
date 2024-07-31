#include "graph.h"

Graph::Graph(const std::vector<Node>& nodes_, const std::vector<Edge>& edges_)
    : nodes(nodes_), edges(edges_) {
    for (const auto& edge : edges) {
        int source = edge.source;
        int target = edge.target;
        int edge_id = &edge - &edges[0]; // Get the index of the edge

        adj_list[source].emplace_back(target, edge_id);
        if (!edge.oneway) {
            adj_list[target].emplace_back(source, edge_id);
        }
    }
}

const std::vector<std::pair<int, int>>& Graph::operator[](int node_id) const {
    auto it = adj_list.find(node_id);
    if (it == adj_list.end()) {
        throw std::out_of_range("Node ID not found in the graph");
    }
    return it->second;
}

bool file_exists_and_readable(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

std::vector<Node> read_nodes(const std::string& filename, std::unordered_map<long long, int>& node_map) {
    std::vector<Node> nodes;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo de nodos " << filename << std::endl;
        return nodes;
    }

    std::string line;
    std::getline(file, line);

    int node_id = 0;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        Node node;

        std::getline(iss, token, ','); // osmid
        long long osmid = std::stoll(token);
        node_map[osmid] = node_id++;
        node.id = node_map[osmid];

        std::getline(iss, token, ','); // y (latitud)
        node.lat = token;
        std::getline(iss, token, ','); // x (longitud)
        node.lon = token;
        std::getline(iss, token, ','); // street_count
        node.street_count = token.empty() ? 0 : std::stoi(token);
        std::getline(iss, token, ','); // highway
        node.highway = token;
        std::getline(iss, token, ','); // geometry
        node.geometry = token;

        nodes.push_back(node);
    }

    std::cout << "Total de nodos: " << nodes.size() << std::endl;
    return nodes;
}

std::vector<Edge> read_edges(const std::string& filename, const std::unordered_map<long long, int>& node_map) {
    std::vector<Edge> edges;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo de aristas " << filename << std::endl;
        return edges;
    }

    std::string line;
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        Edge edge;

        std::getline(iss, token, ','); // source (u)
        edge.source = node_map.at(std::stoll(token));
        std::getline(iss, token, ','); // target (v)
        edge.target = node_map.at(std::stoll(token));
        std::getline(iss, token, ','); // key
        edge.key = token;
        std::getline(iss, token, ','); // osmid
        edge.osmid = token;
        std::getline(iss, token, ','); // oneway
        edge.oneway = (token == "True");
        std::getline(iss, token, ','); // lanes
        edge.lanes = token;
        std::getline(iss, token, ','); // name
        edge.name = token;
        std::getline(iss, token, ','); // highway
        edge.highway = token;
        std::getline(iss, token, ','); // maxspeed
        edge.maxspeed = token;
        std::getline(iss, token, ','); // reversed
        edge.reversed = (token == "True");
        std::getline(iss, token, ','); // length
        edge.length = token;
        std::getline(iss, token, ','); // geometry
        edge.geometry = token;
        std::getline(iss, token, ','); // bridge
        edge.bridge = token;
        std::getline(iss, token, ','); // ref
        edge.ref = token;
        std::getline(iss, token, ','); // junction
        edge.junction = token;
        std::getline(iss, token, ','); // tunnel
        edge.tunnel = token;
        std::getline(iss, token, ','); // width
        edge.width = token;
        std::getline(iss, token, ','); // access
        edge.access = token;
        std::getline(iss, token, ','); // service
        edge.service = token;

        edges.push_back(edge);
    }

    std::cout << "Total de aristas: " << edges.size() << std::endl;
    return edges;
}

Graph build_map(const std::string& nodesfile, const std::string& edgesfile) {
    if (!file_exists_and_readable(nodesfile) || !file_exists_and_readable(edgesfile)) {
        std::cerr << "Error: No se pudo acceder a uno o ambos archivos CSV" << std::endl;
        return Graph({}, {});
    }

    std::unordered_map<long long, int> node_map;
    std::vector<Node> nodes = read_nodes(nodesfile, node_map);
    std::vector<Edge> edges = read_edges(edgesfile, node_map);

    Graph G = Graph(nodes, edges);
    return G;
}
