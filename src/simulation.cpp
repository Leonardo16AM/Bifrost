#include "simulation.h"



simulation::simulation(std::vector<Route>buses_,Graph G_,int habitants=100):G(G_),buses(buses_){

    generate_people(persons, G, habitants); 


    // for (int i = 0; i < H.size(); i++) {
    //     Route sp(graph,to_string(i),{H[i].home_node_id,H[i].work_node_id}, 3);
    //     routes.push_back(sp);
    // }

    // cout<<"PASED ASDASDASDASDASD"<<endl;


}

double simulation::simulate(int days){

    return 1.0;
}

