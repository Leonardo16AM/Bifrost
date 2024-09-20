#include "optimizer.h"

using namespace std;

// Estructura para una celda de la cuadrícula
struct GridCell
{
    std::vector<Node> nodes; // Nodos en esta celda
};

class SpatialGrid
{
public:
    SpatialGrid(double gridSize, const Graph &graph) : gridSize(gridSize)
    {
        // Llenamos la cuadrícula con los nodos del grafo
        for (const auto &node : graph.nodes)
        {
            double lat = node.lat;
            double lon = node.lon;
            auto cell = getCell(lat, lon);
            grid[cell].nodes.push_back(node);
        }
    }

    // Encuentra el nodo más cercano dado latitud y longitud
    int nearestNeighbor(double lat, double lon)
    {
        auto cell = getCell(lat, lon);
        double minDist = std::numeric_limits<double>::max();
        int closestNodeId = -1;

        // Verificamos la celda actual y las celdas vecinas
        for (int i = -1; i <= 1; ++i)
        {
            for (int j = -1; j <= 1; ++j)
            {
                auto neighborCell = std::make_pair(cell.first + i, cell.second + j);
                if (grid.find(neighborCell) != grid.end())
                {
                    for (const auto &node : grid[neighborCell].nodes)
                    {
                        double nodeLat = node.lat;
                        double nodeLon = node.lon;
                        double dist = distance(lat, lon, nodeLat, nodeLon);
                        if (dist < minDist)
                        {
                            minDist = dist;
                            closestNodeId = node.id;
                        }
                    }
                }
            }
        }

        return closestNodeId;
    }

private:
    double gridSize;
    std::map<std::pair<int, int>, GridCell> grid;

    // Función que convierte latitud y longitud a una celda de la cuadrícula
    std::pair<int, int> getCell(double lat, double lon)
    {
        int x = static_cast<int>(lat / gridSize);
        int y = static_cast<int>(lon / gridSize);
        return {x, y};
    }

    // Función para calcular la distancia euclidiana entre dos puntos
    double distance(double lat1, double lon1, double lat2, double lon2)
    {
        return std::sqrt((lat1 - lat2) * (lat1 - lat2) + (lon1 - lon2) * (lon1 - lon2));
    }
};

// Función que devuelve el id del nodo más cercano dada la latitud y longitud
int pointToNode(double lat, double lon, const Graph &graph, bool wtf)
{
    // Definimos el tamaño de la cuadrícula (puede ajustarse según las características del grafo)
    static SpatialGrid spatialGrid(0.01, graph);
    return spatialGrid.nearestNeighbor(lat, lon);
}

// devuelve el id del nodo mas cercano a la latitud y longitud dada
int pointToNode(double lat, double lon, Graph &graph)
{

    // revisar todos los nodos y devolver el qe este mas cerca
    double minDist = 1e9;
    int node = -1;

    for (int i = 0; i < graph.nodes.size(); i++)
    {
        double dist = sqrt((graph.nodes[i].lat - lat) * (graph.nodes[i].lat - lat) + (graph.nodes[i].lon - lon) * (graph.nodes[i].lon - lon));
        if (dist < minDist)
        {
            minDist = dist;
            node = i;
        }
    }

    return node;
}

// void Optimize(const Graph& graph, const vector<Person>& people) {
//     // for(int i=0;i<graph.nodes.size();i++){
//     //     lats.insert({graph.nodes[i].lat, graph.nodes[i].id});
//     //     lons.insert({graph.nodes[i].lon, graph.nodes[i].id});
//     // }

//     // // imprimir la mayor y menor latitud y longitud
//     // cout<<"lats range: "<<lats.begin()->first<<" "<<lats.rbegin()->first<<"\nlon range:"<<lons.begin()->first<<" "<<lons.rbegin()->first<<endl;

//     // cout<<graph.nodes[3].lat<<" "<<graph.nodes[3].lon<<" NODO 3\n";
//     // int nod = pointToNode(22, -82, graph);
//     // cout<<"FOUND NODO "<<nod<<" "<<graph.nodes[nod].lat<<" "<<graph.nodes[nod].lon<<"\n";

// }

int NUM_PARTICLES = 30;
int MAX_ITERATIONS = 100;

set<pair<double, int>> lats, lons;

// Estructura para representar una partícula
struct Particle
{
    std::vector<std::pair<double, double>> positions;      // Pares de coordenadas de inicio y fin de rutas
    std::vector<std::pair<double, double>> velocities;     // Velocidades para cada coordenada
    std::vector<std::pair<double, double>> best_positions; // Mejor posición encontrada por la partícula
    double best_score;
};

// Función para generar un número aleatorio entre dos límites
double random_double(double min, double max)
{
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(min, max);
    return dist(rng);
}

// Implementación de la optimización usando PSO
simulation Optimize(Graph &graph, vector<Person> &people, int number_of_routes,int max_iterations,int num_particles)
{
    int population = people.size();
    simulation best_sim;

    // Parámetros PSO
    double c1 = 1.5, c2 = 2;             // Coeficientes de aceleración: cognitivo y social
    double w = 0.9;                      // Factor de inercia

    // ordenar las latitudes y longitudes
    for (int i = 0; i < graph.nodes.size(); i++)
    {
        lats.insert({graph.nodes[i].lat, graph.nodes[i].id});
        lons.insert({graph.nodes[i].lon, graph.nodes[i].id});
    }
    // imprimir la mayor y menor latitud y longitud
    cout << "lats range from " << lats.begin()->first << " to " << lats.rbegin()->first << "\nlons range from " << lons.begin()->first << " to " << lons.rbegin()->first << endl;

    // Límites del plano (esto puede depender de tu grafo)
    double lat_min = lats.begin()->first, lat_max = lats.rbegin()->first;
    double lon_min = lons.begin()->first, lon_max = lons.rbegin()->first;

    // Inicialización de partículas
    std::vector<Particle> particles(num_particles);
    std::vector<std::pair<double, double>> global_best_position; // Ahora es un vector
    double global_best_score = std::numeric_limits<double>::infinity();

    for (auto &particle : particles)
    {
        for (int i = 0; i < number_of_routes; ++i)
        {
            // Generamos posiciones iniciales aleatorias para cada ruta
            int delta_lat = lat_max - lat_min;
            int delta_lon = lon_max - lon_min;

            double start_lat = random_double(lat_min, lat_max);
            double start_lon = random_double(lon_min, lon_max);
            double end_lat = random_double(lat_min, lat_max);
            double end_lon = random_double(lon_min, lon_max);

            particle.positions.emplace_back(start_lat, start_lon);
            particle.positions.emplace_back(end_lat, end_lon);

            // Velocidades iniciales aleatorias
            double v1 = 0.3;
            particle.velocities.emplace_back(random_double(-v1, v1), random_double(-v1, v1));
            particle.velocities.emplace_back(random_double(-v1, v1), random_double(-v1, v1));
        }
        particle.best_positions = particle.positions;
        particle.best_score = std::numeric_limits<double>::infinity();
    }

    // Bucle principal de PSO
    for (int iteration = 1; iteration <= max_iterations; ++iteration)
    {
        cout << "    PSO ITER: " << iteration << "/" << max_iterations << endl;
        int routecount = 0;
        for (auto &particle : particles)
        {
            cout << "        PARTICLE: " << ++routecount << "/" << num_particles << endl;
            // Crear rutas a partir de las posiciones actuales de la partícula
            // cout << "        DEBUG 1" << endl;
            std::vector<Route> routes;
            double routes_distance_heuristic = 0;
            for (int i = 0; i < number_of_routes; ++i)
            {
                // cout << "        DEBUG 1.1 " << particle.positions.size()<< " " << number_of_routes << endl;
                int start_node = pointToNode(particle.positions[i * 2].first, particle.positions[i * 2].second, graph);
                // cout << "        DEBUG 1.2" << endl;
                int end_node = pointToNode(particle.positions[i * 2 + 1].first, particle.positions[i * 2 + 1].second, graph);
                // cout << "        DEBUG 1.3 " << start_node << " " << end_node << endl;
                Route route = create_route(graph, "route_" + std::to_string(i), start_node, end_node, 1);
                // cout << "        DEBUG 1.4" << endl;
                double dist = sqrt( (pow( (particle.positions[i * 2].first - particle.positions[i * 2 + 1].first), 2) + pow( (particle.positions[i * 2].second - particle.positions[i * 2 + 1].second), 2)) );

                if (route.nodes.size() > 0)
                {
                    routes.push_back(route);
                    routes_distance_heuristic += dist;
                }
                // cout << "        DEBUG 1.5 " << dist << " " << particle.positions[i * 2].first << " " <<particle.positions[i * 2 + 1].first<< " " <<particle.positions[i * 2].second <<" " << particle.positions[i * 2 + 1].second<< " " << endl;
            }
            cout << "        DEBUG 2 " << routes_distance_heuristic << endl;
            if (routes.size() == number_of_routes)
            {
                // Ejecutar simulación con las rutas generadas
                simulation S(routes, graph, people);
                double score = S.simulate() + routes_distance_heuristic*50;

                // Actualizar el mejor valor local de la partícula
                if (score < particle.best_score)
                {
                    particle.best_score = score;
                    particle.best_positions = particle.positions;
                }

                // Actualizar el mejor valor global
                if (score < global_best_score)
                {
                    global_best_score = score;
                    global_best_position = particle.positions; // Ahora asignamos todas las posiciones
                    best_sim = S;
                }
            }
            //  cout << "        DEBUG 3" << endl;
            // Actualizar la velocidad y posición de la partícula
            for (int i = 0; i < number_of_routes * 2; ++i)
            {
                particle.velocities[i].first = w * particle.velocities[i].first + c1 * random_double(0, 1) * (particle.best_positions[i].first - particle.positions[i].first) + c2 * random_double(0, 1) * (global_best_position[i].first - particle.positions[i].first);

                particle.velocities[i].second = w * particle.velocities[i].second + c1 * random_double(0, 1) * (particle.best_positions[i].second - particle.positions[i].second) + c2 * random_double(0, 1) * (global_best_position[i].second - particle.positions[i].second);

                // Actualizar posición
                particle.positions[i].first += particle.velocities[i].first;
                if (particle.positions[i].first > lat_max)
                    particle.positions[i].first = lat_max;
                if (particle.positions[i].first < lat_min)
                    particle.positions[i].first = lat_min;

                particle.positions[i].second += particle.velocities[i].second;
                if (particle.positions[i].second > lon_max)
                    particle.positions[i].second = lon_max;
                if (particle.positions[i].second < lon_min)
                    particle.positions[i].second = lon_min;
            }
            // cout << "        DEBUG 4" << endl;
        }

        // imprimir el mejor resultado hasta ahora
        cout << "GLOBAL BEST: " << global_best_score << endl;

        if (iteration % 5 == 0)
        {
            cout << "SAVING SIMULATION\n";
            best_sim.save_simulation_to_csv("best_simulation.csv");
        }
    }

    // Devolver las mejores rutas encontradas
    std::vector<Route> best_routes;
    for (int i = 0; i < number_of_routes; ++i)
    {
        int start_node = pointToNode(global_best_position[i * 2].first, global_best_position[i * 2].second, graph);
        int end_node = pointToNode(global_best_position[i * 2 + 1].first, global_best_position[i * 2 + 1].second, graph);
        Route route = create_route(graph, "route_" + std::to_string(i), start_node, end_node, 1);
        best_routes.push_back(route);
    }

    // return best_routes;
    return best_sim;
}