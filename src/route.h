#ifndef ROUTE_H
#define ROUTE_H

#include <vector>
#include <string>
#include <SFML/Graphics.hpp>

class Route {
public:
    Route(const std::string& id_, const std::vector<int>& stops_, const std::vector<int>& streets_, int bus_count_, double total_distance_=0.0, const sf::Color& color_ = sf::Color::Yellow);

    std::string id;
    std::vector<int> stops; 
    std::vector<int> streets; 
    int bus_count;
    double total_distance; 
    sf::Color color;
};

#endif // ROUTE_H
