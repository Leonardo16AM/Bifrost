#ifndef SIMULATION_H
#define SIMULATION_H
#include "graph.h"
#include "route.h"
#include "person.h"
#include "visualization.h"
#include <vector>



class simulation{
public:
    simulation(std::vector<Route>buses_,Graph G_,int habitants);
    double simulate(int days=7);
    double recursive_sim(std::vector<Person>&subset_pers,std::unordered_set<int> &visitable_nodes );
private:
    Graph G;
    Graph BG;
    std::vector<Person>persons;
    std::vector<Route>buses;
};


#endif