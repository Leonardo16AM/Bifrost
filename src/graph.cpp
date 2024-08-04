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


void Graph::calculate_betweenness_centrality() {
    std::vector<double> betweenness(nodes.size(), 0.0);
    
    for (size_t s = 0; s < nodes.size(); ++s) {
        std::stack<int> S;
        std::vector<std::vector<int>> P(nodes.size());
        std::vector<int> sigma(nodes.size(), 0);
        std::vector<int> d(nodes.size(), -1);
        std::queue<int> Q;
        
        sigma[s] = 1;
        d[s] = 0;
        Q.push(s);
        
        while (!Q.empty()) {
            int v = Q.front(); Q.pop();
            S.push(v);
            
            for (const auto& neighbor : adj_list[v]) {
                int w = neighbor.first;
                
                if (d[w] < 0) {
                    Q.push(w);
                    d[w] = d[v] + 1;
                }
                
                if (d[w] == d[v] + 1) {
                    sigma[w] += sigma[v];
                    P[w].push_back(v);
                }
            }
        }
        
        std::vector<double> delta(nodes.size(), 0.0);
        
        while (!S.empty()) {
            int w = S.top(); S.pop();
            for (int v : P[w]) {
                delta[v] += (static_cast<double>(sigma[v]) / sigma[w]) * (1.0 + delta[w]);
            }
            if (w != s) {
                betweenness[w] += delta[w];
            }
        }
    }
    
    for (size_t i = 0; i < nodes.size(); ++i) {
        nodes[i].betweenness_centrality = betweenness[i];
    }
    save_betweenness_to_csv("maps/betweenness.csv");
}

void Graph::save_betweenness_to_csv(const std::string& filename) const {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "id,betweenness_centrality\n";
        for (const auto& node : nodes) {
            file << node.id << "," << node.betweenness_centrality << "\n";
        }
        file.close();
    } else {
        std::cerr << "Error opening file for writing: " << filename << "\n";
    }
}


void Graph::load_betweenness_from_csv(const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        std::getline(file, line); // Leer encabezado
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string id_str, centrality_str;
            if (std::getline(iss, id_str, ',') && std::getline(iss, centrality_str, ',')) {
                int id = std::stoi(id_str);
                double centrality = std::stod(centrality_str);
                auto it = std::find_if(nodes.begin(), nodes.end(), [id](const Node& node) {
                    return node.id == id;
                });
                if (it != nodes.end()) {
                    it->betweenness_centrality = centrality;
                }
            }
        }
        file.close();
    } else {
        std::cerr << "Error opening file for reading: " << filename << "\n";
    }
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



std::vector<int> Graph::a_star(int start_id, int goal_id) const {
    // Verificar si los nodos de inicio y meta existen
    if (start_id < 0 || start_id >= nodes.size() || goal_id < 0 || goal_id >= nodes.size()) {
        std::cerr << "Error: Nodo de inicio o meta no existe en el mapa." << std::endl;
        return {};
    }

    std::unordered_set<int> closed_set;
    std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, std::greater<>> open_set;
    std::unordered_map<int, int> came_from;
    std::unordered_map<int, double> g_score;
    std::unordered_map<int, double> f_score;

    for (const auto& node : nodes) {
        g_score[node.id] = std::numeric_limits<double>::infinity();
        f_score[node.id] = std::numeric_limits<double>::infinity();
    }

    g_score[start_id] = 0.0;
    f_score[start_id] = heuristic(start_id, goal_id);
    open_set.emplace(f_score[start_id], start_id);

    while (!open_set.empty()) {
        int current = open_set.top().second;
        open_set.pop();

        if (current == goal_id) {
            std::vector<int> path;
            while (came_from.find(current) != came_from.end()) {
                path.push_back(current);
                current = came_from[current];
            }
            path.push_back(start_id);
            std::reverse(path.begin(), path.end());
            return path;
        }

        closed_set.insert(current);

        // Verificar si el nodo actual tiene vecinos en el mapa
        if (adj_list.find(current) == adj_list.end()) continue;

        for (const auto& [neighbor, length] : adj_list.at(current)) {
            if (closed_set.find(neighbor) != closed_set.end()) continue;

            double tentative_g_score = g_score[current] + length;
            if (tentative_g_score < g_score[neighbor]) {
                came_from[neighbor] = current;
                g_score[neighbor] = tentative_g_score;
                f_score[neighbor] = g_score[neighbor] + heuristic(neighbor, goal_id);

                // Evitar agregar el mismo nodo nuevamente si ya está en el conjunto abierto con un mejor costo
                bool in_open_set = false;
                auto tmp_queue = open_set; // Crear una copia temporal para buscar
                while (!tmp_queue.empty()) {
                    if (tmp_queue.top().second == neighbor) {
                        in_open_set = true;
                        break;
                    }
                    tmp_queue.pop();
                }
                
                if (!in_open_set)
                    open_set.emplace(f_score[neighbor], neighbor);
            }
        }
    }

    std::cerr << "No se encontró un camino." << std::endl;
    return {}; // Retornar un camino vacío si no se encuentra una ruta
}

double Graph::heuristic(int node_id1, int node_id2) const {
    auto lat1 = std::stod(nodes[node_id1].lat);
    auto lon1 = std::stod(nodes[node_id1].lon);
    auto lat2 = std::stod(nodes[node_id2].lat);
    auto lon2 = std::stod(nodes[node_id2].lon);
    return std::sqrt(std::pow(lat2 - lat1, 2) + std::pow(lon2 - lon1, 2));
}



std::unordered_map<int, std::pair<int, double>> Graph::dijkstra(int start_id, const std::unordered_set<int>& visitable_nodes) const {
    std::unordered_map<int, std::pair<int, double>> results; // nodo -> (nodo previo, distancia mínima)
    std::set<std::pair<double, int>> queue; // (distancia, nodo)

    // Inicializar las distancias a infinito y el nodo previo a -1
    for (int node_id : visitable_nodes) {
        results[node_id] = {-1, std::numeric_limits<double>::infinity()};
    }
    results[start_id] = {-1, 0};
    queue.insert({0, start_id});

    while (!queue.empty()) {
        auto [dist, current] = *queue.begin();
        queue.erase(queue.begin());

        // Recorrer los vecinos
        if (adj_list.find(current) != adj_list.end()) {
            for (auto [neighbour, weight] : adj_list.at(current)) {
                if (visitable_nodes.find(neighbour) != visitable_nodes.end()) {
                    double new_dist = dist + weight;
                    if (new_dist < results[neighbour].second) {
                        queue.erase({results[neighbour].second, neighbour});
                        results[neighbour] = {current, new_dist};
                        queue.insert({new_dist, neighbour});
                    }
                }
            }
        }
    }

    return results;
}



std::vector<int> Graph::reconstruct_path(int start_id, int goal_id, const std::unordered_map<int, std::pair<int, double>>& dijkstra_result) const {
    std::vector<int> path;
    int current = goal_id;
    
    // Rastrea el camino de vuelta desde el nodo final al inicial
    while (current != start_id) {
        path.push_back(current);
        auto it = dijkstra_result.find(current);
        if (it == dijkstra_result.end() || it->second.first == -1) {
            // Si no encuentra un nodo previo o el nodo previo es -1, el camino no es válido
            return {}; // Devuelve un vector vacío si no hay camino válido
        }
        current = it->second.first; // Actualiza el nodo actual al nodo previo
    }
    path.push_back(start_id); // Añade el nodo inicial al final del camino
    std::reverse(path.begin(), path.end()); // Invierte el vector para obtener el camino en el orden correcto desde el inicio hasta el final

    return path; // Devuelve el camino reconstruido
}