#include "route.h"

Route::Route(Graph &graph, const std::string &id_, const std::vector<int> &stops_, int bus_count_, double total_distance_, const sf::Color &color_)
    : id(id_), stops(stops_), bus_count(bus_count_), total_distance(total_distance_), color(color_)
{

    std::unordered_set<int> visitable_nodes;
    for (int i = 0; i <= graph.nodes.size(); i++)
    {
        visitable_nodes.insert(i);
    }
    for (int i = 1; i < stops_.size(); i++)
    {
        auto dijkstra_result = graph.dijkstra(stops_[i - 1], visitable_nodes);
        std::vector<int> shortest_path = graph.reconstruct_path(stops_[i - 1], stops_[i], dijkstra_result);

        for (auto it : shortest_path)
        {
            nodes.push_back(it);
        }
    }
}

Route create_route(Graph &graph, string id, int starting_node, int ending_node, int bus_count)
{
    Route sp(graph, id, {starting_node, ending_node}, bus_count);
    // Hacemos que cada 5 calles tengamos una parada
    sp.stops.pop_back();
    if (sp.nodes.size() == 0)
        return sp;
    for (int i = 0; i < sp.nodes.size() - 1; i++)
    {
        if (i % 5 == 0)
        {
            sp.stops.push_back(sp.nodes[i]);
        }
    }
    sp.stops.push_back(sp.nodes.back());
    return sp;
}