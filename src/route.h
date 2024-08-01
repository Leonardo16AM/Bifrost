#ifndef ROUTE_H
#define ROUTE_H

#include <vector>
#include <string>
#include <SFML/Graphics.hpp>

class Route {
public:
    Route(const std::string& id_, const std::vector<int>& stops_, const std::vector<int>& streets_, int bus_count_, double total_distance_, const sf::Color& color_ = sf::Color::Yellow);

    std::string id; // ID de la ruta
    std::vector<int> stops; // Lista de enteros para nodos de las paradas
    std::vector<int> streets; // Lista de enteros para nodos consecutivos de las calles
    int bus_count; // Número de ómnibus que pasan por la ruta
    double total_distance; // Distancia total de la ruta
    sf::Color color; // Color de la ruta
};

#endif // ROUTE_H
