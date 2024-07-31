#include "route.h"

Route::Route(const std::string& id_, const std::vector<int>& stops_, const std::vector<int>& streets_, int bus_count_, double total_distance_, const sf::Color& color_)
    : id(id_), stops(stops_), streets(streets_), bus_count(bus_count_), total_distance(total_distance_), color(color_) {}

const std::string& Route::get_id() const {
    return id;
}

const std::vector<int>& Route::get_stops() const {
    return stops;
}

const std::vector<int>& Route::get_streets() const {
    return streets;
}

int Route::get_bus_count() const {
    return bus_count;
}

double Route::get_total_distance() const {
    return total_distance;
}

const sf::Color& Route::get_color() const {
    return color;
}

void Route::add_stop(int node) {
    stops.push_back(node);
}

void Route::add_street(int node) {
    streets.push_back(node);
}

void Route::set_bus_count(int count) {
    bus_count = count;
}

void Route::set_total_distance(double distance) {
    total_distance = distance;
}

void Route::set_color(const sf::Color& color_) {
    color = color_;
}
