#include "route.h"

Route::Route(Graph &graph,const std::string& id_, const std::vector<int>& stops_,  int bus_count_, double total_distance_, const sf::Color& color_)
    : id(id_), stops(stops_), bus_count(bus_count_), total_distance(total_distance_), color(color_) {

        std::unordered_set<int> visitable_nodes;
        for(int i=0;i<=graph.nodes.size();i++){
            visitable_nodes.insert(i);
        }
        for(int i=1;i<stops_.size();i++){            
            auto dijkstra_result = graph.dijkstra(stops_[i-1], visitable_nodes);
            std::vector<int> shortest_path = graph.reconstruct_path(stops_[i-1], stops_[i], dijkstra_result);
            
            for(auto it: shortest_path){
                nodes.push_back(it);
            }        
        }

}

