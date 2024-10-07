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

using namespace std;

enum class event_type {
    PERSON_START_WALKING,
    PERSON_ARRIVE_STOP,
    BUS_ARRIVE_STOP,
    PERSON_GET_OFF_BUS,
    PERSON_ARRIVE_WORK
};

struct event {
    double time; 
    event_type type;
    int entity_id;

    bool operator<(const event& other) const ;
};

struct sim_person {
    int id;
    vector<int>path_nodes;
    vector<double>distances;
    int current_stop=0;

    double move();
    bool on_work();
};

struct sim_bus{
    int route_id;
    int id;
    vector<int> stops; 
    vector<double> distances; 
   
    int current_stop=0;
    set<int>visited;
    map<int,vector<int>>passengers;

    double move();
    vector<int>leave_on_stop();
    bool on_direction(int node);
};

class simulation{
public:
    simulation(vector<Route> buses_, Graph G_, vector<Person> &persons_,vector<double>&cost_per_person);
    simulation();
    double simulate_person(Person &person, unordered_set<int> &visitable_nodes);
    vector<double>simulate_persons(vector<Person> &subset_pers, unordered_set<int> &visitable_nodes);

    double simulate(int days = 7);
    double average(vector<double> &vals);
    double CVaR90(vector<Person> &subset_pers, unordered_set<int> &visitable_nodes);
    vector<Route> get_routes();
    vector<Person> get_people();
    void save_simulation_to_csv(const string &filename) const;
    void load_simulation_from_csv(const string &filename);
private:
    Graph G;
    Graph BG;
    vector<Person> persons;
    vector<Route> buses;
    vector<sim_person>sim_persons;
    vector<sim_bus>sim_buses;
};

#endif