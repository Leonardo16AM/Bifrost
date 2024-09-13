#ifndef ROUTE_H
#define ROUTE_H

#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include "graph.h"
#include <unordered_set>

using namespace std;

class Route {
public:
    Route(Graph &graph,const std::string& id_, const std::vector<int>& stops_, int bus_count_, double total_distance_=0.0, const sf::Color& color_ = sf::Color::Yellow);

    std::string id;
    std::vector<int> stops; 
    std::vector<int> nodes; 
    int bus_count;
    double total_distance; 
    sf::Color color;
};

Route create_route(Graph &graph, std::string id, int starting_node, int ending_node, int bus_count);

#endif // ROUTE_H
