#include "simulation.h"
#include "unordered_map"
using namespace std;


simulation::simulation(std::vector<Route>buses_,Graph G_,int habitants=100):G(G_),buses(buses_){

    generate_people(persons, G, habitants); 
    
    vector<Node>BN=G.nodes;
    vector<Edge>BE=G.edges;
    
    for(auto &e: BE){
        e.length=e.length/84.0; // minutos caminando
    }

    int idn=BN.size();

    for(auto b:buses){
        for(int i=0;i<b.stops.size();i++){
            
            Node node_wr=BN[b.stops[i]];
            BN.push_back(node_wr);

            Edge ne;
            ne.source=b.stops[i];
            ne.target=idn;
            ne.oneway=true;
            ne.length=10;
            BE.push_back(ne);
            
            ne.source=idn;
            ne.target=b.stops[i];
            ne.oneway=true;
            ne.length=0;
            BE.push_back(ne);

            if(i==0)continue;

            
            ne.source=idn-1;
            ne.target=idn;
            ne.oneway=false;
            ne.length=2.5;

            BE.push_back(ne);
            
            idn++;
        }
    }

    BG=Graph(BN,BE);
}

double simulation::simulate(int days){
    unsigned long long sum=0;
    int num_trips=0;

    std::unordered_set<int> visitable_nodes;
    for(int i=0;i<=BG.nodes.size();i++){
        visitable_nodes.insert(i);
    }

    for(int i=0;i<persons.size();i++){
        std::unordered_map<int, std::pair<int, double>>um = BG.dijkstra(persons[i].home_node_id, visitable_nodes);
        long double sp=um[persons[i].work_node_id].second;
        sum+=sp;
        num_trips++;
    }
    return (long double)sum/(long double)num_trips;
}

