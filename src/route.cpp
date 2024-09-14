#include "route.h"

Route::Route(Graph &graph, const std::string &id_, const std::vector<int> &stops_, int bus_count_, double total_distance_, const sf::Color &color_)
    : id(id_), stops(stops_), bus_count(bus_count_), total_distance(total_distance_), color(color_)
{
    // std::cout<<"        DEBUG ESP.1 "<<graph.nodes.size()<<std::endl;
    std::unordered_set<int> visitable_nodes;
    for (int i = 0; i <= graph.nodes.size(); i++)
    {
        // std::cout<<"        DEBUG ESP.1."<<i<<std::endl;
        visitable_nodes.insert(i);
    }
    // std::cout<<"        DEBUG ESP.2 "<<std::endl;
    for (int i = 1; i < stops_.size(); i++)
    {
        auto dijkstra_result = graph.dijkstra(stops_[i - 1], visitable_nodes);
        
        if(dijkstra_result[stops[i]].second==std::numeric_limits<double>::infinity()){continue;}
        
        std::vector<int> shortest_path = graph.reconstruct_path(stops_[i - 1], stops_[i], dijkstra_result);


        for (auto it : shortest_path)
        {
            nodes.push_back(it);
        }
    }
    // std::cout<<"        DEBUG ESP.3 "<<std::endl;
}

Route create_route(Graph &graph, std::string id, int starting_node, int ending_node, int bus_count)
{
    // std::cout<<"        DEBUG 1.3.1 "<<std::endl;
    Route sp(graph, id, {starting_node, ending_node}, bus_count);
    // std::cout<<"        DEBUG 1.3.2 "<<std::endl;
    // Hacemos que cada 5 calles tengamos una parada
    sp.stops.pop_back();
    // std::cout<<"        DEBUG 1.3.3 "<<std::endl;
    if (sp.nodes.size() == 0)
    {
        // std::cout<<"        DEBUG 1.3.ALT "<<std::endl;
        return sp;
    }
    // std::cout<<"        DEBUG 1.3.4 "<<std::endl;
    for (int i = 0; i < sp.nodes.size() - 1; i++)
    {
        if (i % 5 == 0)
        {
            sp.stops.push_back(sp.nodes[i]);
        }
    }
    // std::cout<<"        DEBUG 1.3.5 "<<std::endl;
    sp.stops.push_back(sp.nodes.back());
    // std::cout<<"        DEBUG 1.3.6 "<<std::endl;
    return sp;
}