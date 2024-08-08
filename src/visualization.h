#pragma once

#ifdef _WIN32
#include <windows.h>
#include <dwmapi.h>
#else
#include <unistd.h>
#endif

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <random>
#include "graph.h"
#include "route.h"
#include "person.h"

#pragma comment(lib, "Dwmapi.lib")

struct NormalizedNode {
    sf::Vector2f position;
    NormalizedNode(float x, float y) : position(x, y) {}
};


#ifdef _WIN32
void set_window_title_bar_color(sf::RenderWindow& window, COLORREF color);
#endif

void find_min_max_lat_lon(const Graph& graph, float& minLat, float& maxLat, float& minLon, float& maxLon);
sf::Vector2f normalize_coordinates(float lat, float lon, float minLat, float maxLat, float minLon, float maxLon, float width, float height);
void precompute_normalized_coordinates(const Graph& graph, std::vector<NormalizedNode>& normalizedNodes, float minLat, float maxLat, float minLon, float maxLon, float width, float height);
void draw_graph(sf::RenderWindow& window, const Graph& graph, const std::vector<NormalizedNode>& normalizedNodes);
void draw_people(sf::RenderWindow& window, const std::vector<Person>& people, const std::vector<NormalizedNode>& normalizedNodes, const std::vector<Edge>& edges);
float euclidean_distance(const sf::Vector2f& a, const sf::Vector2f& b);
void draw_text_with_outline(sf::RenderWindow& window, sf::Text& text, sf::Color outlineColor, float thickness = 2.f);
void draw_routes(sf::RenderWindow& window, const std::vector<Route>& routes, const std::vector<NormalizedNode>& normalizedNodes);
void display_route_properties(sf::RenderWindow& window, const Route& route, const sf::Font& font, float windowWidth);
void generate_people(std::vector<Person>& people, const Graph& graph, int numPeople);
sf::Color generate_light_color();
void draw_partitioned_nodes(sf::RenderWindow& window, const std::vector<std::pair<int, int>>& node_partition, const std::vector<NormalizedNode>& normalizedNodes);
void draw_mincut(sf::RenderWindow& window, const std::vector<Edge>& mincut_edges, const std::vector<NormalizedNode>& normalizedNodes);