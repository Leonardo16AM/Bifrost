#include "simulation.h"
#include "unordered_map"
#include <random>
using namespace std;


simulation::simulation(std::vector<Route>buses_,Graph G_,int habitants=100):G(G_),buses(buses_){

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);


    generate_people(persons, G, habitants); 
    
    vector<Node>BN=G.nodes;
    vector<Edge>BE=G.edges;
    
    for(auto &e: BE){
        e.length=e.length/84.0;
    }

    int idn=BN.size();

    for(auto b:buses){
        double route_time=5*b.nodes.size();
        for(int i=0;i<b.stops.size();i++){
            
            Node node_wr=BN[b.stops[i]];
            BN.push_back(node_wr);

            Edge ne;
            ne.source=b.stops[i];
            ne.target=idn;
            ne.oneway=true;
            ne.length=(route_time* dis(gen) )/(double)b.bus_count;
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
            ne.length=1;

            BE.push_back(ne);
            
            idn++;
        }
    }

    BG=Graph(BN,BE);
}

double simulation::recursive_sim(std::vector<Person>&subset_pers,std::unordered_set<int> &visitable_nodes ){
    if( subset_pers.size()==1 ){
        std::unordered_map<int, std::pair<int, double>>um = BG.dijkstra(subset_pers[0].home_node_id, visitable_nodes);
        long double sp=um[subset_pers[0].work_node_id].second;
        return sp; 
    }


    vector<Person>L,R;
    for(int i=0;i<subset_pers.size()/2;i++){
        L.push_back(subset_pers[i]);
    }
    for(int i=subset_pers.size()/2;i<subset_pers.size();i++){
        R.push_back(subset_pers[i]);
    }
    double dl=recursive_sim(L,visitable_nodes );
    double dr=recursive_sim(R,visitable_nodes );
    return (double)(dl+dr)/2.0;
}

double simulation::simulate(int days){
    unsigned long long sum=0;
    int num_trips=0;

    std::unordered_set<int> visitable_nodes;
    for(int i=0;i<=BG.nodes.size();i++){
        visitable_nodes.insert(i);
    }

    return recursive_sim(persons,visitable_nodes );
}

