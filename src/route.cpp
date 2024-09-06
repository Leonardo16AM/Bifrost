#include "route.h"

Route::Route(const std::string& id_, const std::vector<int>& stops_, const std::vector<int>& nodes_, int bus_count_, double total_distance_, const sf::Color& color_)
    : id(id_), stops(stops_), nodes(nodes_), bus_count(bus_count_), total_distance(total_distance_), color(color_) {}

