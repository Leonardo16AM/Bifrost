#ifndef ROUTE_H
#define ROUTE_H

#include <vector>
#include <string>
#include <SFML/Graphics.hpp>

class Route {
public:
    Route(const std::string& id_, const std::vector<int>& stops_, const std::vector<int>& streets_, int bus_count_, double total_distance_, const sf::Color& color_ = sf::Color::Yellow);

    const std::string& get_id() const;
    const std::vector<int>& get_stops() const;
    const std::vector<int>& get_streets() const;
    int get_bus_count() const;
    double get_total_distance() const;
    const sf::Color& get_color() const;

    void add_stop(int node);
    void add_street(int node);
    void set_bus_count(int count);
    void set_total_distance(double distance);
    void set_color(const sf::Color& color_);

    std::string id; // ID de la ruta
    std::vector<int> stops; // Lista de enteros para nodos de las paradas
    std::vector<int> streets; // Lista de enteros para nodos consecutivos de las calles
    int bus_count; // Número de ómnibus que pasan por la ruta
    double total_distance; // Distancia total de la ruta
    sf::Color color; // Color de la ruta
};

#endif // ROUTE_H
