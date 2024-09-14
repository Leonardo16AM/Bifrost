#include "route.h"

Route::Route(Graph &graph, const std::string &id_, const std::vector<int> &stops_, int bus_count_, double total_distance_, const sf::Color &color_)
    : id(id_), stops(stops_), bus_count(bus_count_), total_distance(total_distance_), color(color_)
{
    cout<<"        DEBUG ESP.1 "<<graph.nodes.size()<<endl;
    std::unordered_set<int> visitable_nodes;
    for (int i = 0; i < graph.nodes.size(); i++)
    {
        visitable_nodes.insert(i);
    }
    cout<<"        DEBUG ESP.2 "<<endl;
    for (int i = 1; i < stops_.size(); i++)
    {
        auto dijkstra_result = graph.dijkstra(stops_[i - 1], visitable_nodes);
        std::vector<int> shortest_path = graph.reconstruct_path(stops_[i - 1], stops_[i], dijkstra_result);

        for (auto it : shortest_path)
        {
            nodes.push_back(it);
        }
    }
    cout<<"        DEBUG ESP.3 "<<endl;
}

Route create_route(Graph &graph, string id, int starting_node, int ending_node, int bus_count)
{
    cout<<"        DEBUG 1.3.1 "<<endl;
    Route sp(graph, id, {starting_node, ending_node}, bus_count);
    cout<<"        DEBUG 1.3.2 "<<endl;
    // Hacemos que cada 5 calles tengamos una parada
    sp.stops.pop_back();
    cout<<"        DEBUG 1.3.3 "<<endl;
    if (sp.nodes.size() == 0)
    {
        cout<<"        DEBUG 1.3.ALT "<<endl;
        return sp;
    }
    cout<<"        DEBUG 1.3.4 "<<endl;
    for (int i = 0; i < sp.nodes.size() - 1; i++)
    {
        if (i % 5 == 0)
        {
            sp.stops.push_back(sp.nodes[i]);
        }
    }
    cout<<"        DEBUG 1.3.5 "<<endl;
    sp.stops.push_back(sp.nodes.back());
    cout<<"        DEBUG 1.3.6 "<<endl;
    return sp;
}