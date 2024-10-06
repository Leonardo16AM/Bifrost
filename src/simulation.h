#ifndef SIMULATION_H
#define SIMULATION_H
#include "graph.h"
#include "route.h"
#include "person.h"
#include "visualization.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

class simulation
{
public:
    // simulation(std::vector<Route> buses_, Graph G_, int habitants);
    simulation(std::vector<Route> buses_, Graph G_, std::vector<Person> persons_);
    simulation();
    double simulate_person(Person &person, std::unordered_set<int> &visitable_nodes);
    std::vector<double>simulate_persons(std::vector<Person> &subset_pers, std::unordered_set<int> &visitable_nodes);

    double simulate(int days = 7);
    double average(std::vector<double> &vals);
    double CVaR90(std::vector<Person> &subset_pers, std::unordered_set<int> &visitable_nodes);
    std::vector<Route> get_routes();
    std::vector<Person> get_people();
    void save_simulation_to_csv(const std::string &filename) const;
    void load_simulation_from_csv(const std::string &filename);
private:
    Graph G;
    Graph BG;
    std::vector<Person> persons;
    std::vector<Route> buses;
};

#endif