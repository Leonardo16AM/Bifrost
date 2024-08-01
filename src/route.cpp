#include "route.h"

Route::Route(const std::string& id_, const std::vector<int>& stops_, const std::vector<int>& streets_, int bus_count_, double total_distance_, const sf::Color& color_)
    : id(id_), stops(stops_), streets(streets_), bus_count(bus_count_), total_distance(total_distance_), color(color_) {}

