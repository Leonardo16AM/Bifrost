#include "optimizer.h"

using namespace std;


vector<pair<double, double>> interpolate(pair<double, double> start_point, pair<double, double> end_point, int num_intermediate_points) 
{
    vector<pair<double, double>> points;
    points.push_back(start_point); // Agregar el punto inicial
    
    double delta_x = (end_point.first - start_point.first) / (num_intermediate_points + 1);
    double delta_y = (end_point.second - start_point.second) / (num_intermediate_points + 1);
    
    for (int i = 1; i <= num_intermediate_points; ++i) {
        double x = start_point.first + i * delta_x;
        double y = start_point.second + i * delta_y;
        points.emplace_back(x, y); // Agregar los puntos intermedios
    }
    
    points.push_back(end_point); // Agregar el punto final
    return points;
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


int NUM_PARTICLES = 30;
int MAX_ITERATIONS = 100;
int ROUTE_BREAKPOINTS = 5;

set<pair<double, int>> lats, lons;

// Función para generar un número aleatorio entre dos límites
double random_double(double min, double max)
{
    static mt19937 rng(random_device{}());
    uniform_real_distribution<double> dist(min, max);
    return dist(rng);
}

// Estructura para representar una partícula
struct Particle
{
    vector<vector<pair<double, double>>> positions;
    vector<vector<pair<double, double>>> velocities;
    vector<vector<pair<double, double>>> best_positions;
    // vector<pair<double, double>> positions;      // Pares de coordenadas de inicio y fin de rutas
    // vector<pair<double, double>> velocities;     // Velocidades para cada coordenada
    // vector<pair<double, double>> best_positions; // Mejor posición encontrada por la partícula
    double best_score;
};

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
    // cout << "lats range from " << lats.begin()->first << " to " << lats.rbegin()->first << "\nlons range from " << lons.begin()->first << " to " << lons.rbegin()->first << endl;

    // Límites del plano (esto puede depender de tu grafo)
    double lat_min = lats.begin()->first, lat_max = lats.rbegin()->first;
    double lon_min = lons.begin()->first, lon_max = lons.rbegin()->first;
    double delta_lat = lat_max - lat_min;
    double delta_lon = lon_max - lon_min;

    // Inicialización de partículas
    vector<Particle> particles(num_particles);
    vector<vector<pair<double, double>>> global_best_position; // Ahora es un vector
    double global_best_score = numeric_limits<double>::infinity();

    for (auto &particle : particles)
    {
        for (int i = 0; i < number_of_routes; ++i)
        {
            // Generamos posiciones iniciales aleatorias para cada ruta

            double start_lat = random_double(lat_min, lat_max);
            double start_lon = random_double(lon_min, lon_max);
            double end_lat = random_double(lat_min, lat_max);
            double end_lon = random_double(lon_min, lon_max);

            // Interpolamos puntos intermedios
            particle.positions.push_back(interpolate({start_lat, start_lon}, {end_lat, end_lon}, ROUTE_BREAKPOINTS));

            // Velocidades iniciales aleatorias
            double v1 = 0.3;
            particle.velocities.push_back({});
            for(int j = 0; j < ROUTE_BREAKPOINTS + 2; ++j)
            {
                particle.velocities[i].push_back({random_double(-v1, v1), random_double(-v1, v1)});
            }
        }
        particle.best_positions = particle.positions;
        particle.best_score = numeric_limits<double>::infinity();
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
            vector<Route> routes;
            double routes_distance_heuristic = 0;
            for (int i = 0; i < number_of_routes; ++i)
            {
                // cout << "        DEBUG 1.1 " << particle.positions.size()<< " " << number_of_routes << endl;
                int start_node = pointToNode(particle.positions[i][0].first, particle.positions[i][0].second, graph);
                // cout << "        DEBUG 1.2" << endl;
                int end_node = pointToNode(particle.positions[i][ROUTE_BREAKPOINTS+1].first, particle.positions[i][ROUTE_BREAKPOINTS].second, graph);
                // cout << "        DEBUG 1.3 " << start_node << " " << end_node << endl;
                Route route = create_route(graph, "route_" + to_string(i), start_node, end_node, 5);
                // cout << "        DEBUG 1.4" << endl;
                // double dist = sqrt( (pow( (particle.positions[i * 2].first - particle.positions[i * 2 + 1].first), 2) + pow( (particle.positions[i * 2].second - particle.positions[i * 2 + 1].second), 2)) );

                if (route.nodes.size() > 0)
                {
                    routes.push_back(route);
                    // routes_distance_heuristic += dist;
                }
                // cout << "        DEBUG 1.5 " << dist << " " << particle.positions[i * 2].first << " " <<particle.positions[i * 2 + 1].first<< " " <<particle.positions[i * 2].second <<" " << particle.positions[i * 2 + 1].second<< " " << endl;
            }
            // cout << "        DEBUG 2 " << routes_distance_heuristic << endl;
            if (routes.size() == number_of_routes)
            {
                // Ejecutar simulación con las rutas generadas
                vector<double>emp;
                simulation S(routes, graph, people,emp);
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
            for (int i = 0; i < number_of_routes; ++i)
            {
                for(int j = 0; j < ROUTE_BREAKPOINTS + 2; ++j)
                {
                    // cout << "        DEBUG 3.1" << endl;
                    // Actualizar velocidad
                    particle.velocities[i][j].first = w * particle.velocities[i][j].first + c1 * random_double(0, 1) * (particle.best_positions[i][j].first - particle.positions[i][j].first) + c2 * random_double(0, 1) * (global_best_position[i][j].first - particle.positions[i][j].first);

                    particle.velocities[i][j].second = w * particle.velocities[i][j].second + c1 * random_double(0, 1) * (particle.best_positions[i][j].second - particle.positions[i][j].second) + c2 * random_double(0, 1) * (global_best_position[i][j].second - particle.positions[i][j].second);

                    // Actualizar posición
                    particle.positions[i][j].first += particle.velocities[i][j].first;
                    if (particle.positions[i][j].first > lat_max)
                        particle.positions[i][j].first = lat_max;
                    if (particle.positions[i][j].first < lat_min)
                        particle.positions[i][j].first = lat_min;

                    particle.positions[i][j].second += particle.velocities[i][j].second;
                    if (particle.positions[i][j].second > lon_max)
                        particle.positions[i][j].second = lon_max;
                    if (particle.positions[i][j].second < lon_min)
                        particle.positions[i][j].second = lon_min;
                }
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
    // vector<Route> best_routes;
    // for (int i = 0; i < number_of_routes; ++i)
    // {
    //     int start_node = pointToNode(global_best_position[i * 2].first, global_best_position[i * 2].second, graph);
    //     int end_node = pointToNode(global_best_position[i * 2 + 1].first, global_best_position[i * 2 + 1].second, graph);
    //     Route route = create_route(graph, "route_" + to_string(i), start_node, end_node, 5);
    //     best_routes.push_back(route);
    // }

    // return best_routes;
    return best_sim;
}