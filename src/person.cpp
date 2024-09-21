#include "person.h"
#include <cmath>
#include <iostream>
#include <random>

Person::Person(){}

Person::Person(int home_node_id_, int work_node_id_)
    : home_node_id(home_node_id_), work_node_id(work_node_id_){
    

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    std::normal_distribution<> distribution(85, 10); 


    phisical_state=dis(gen);
    patience=dis(gen);
    money=dis(gen);
    do {
        speed = distribution(gen);
    } while (speed < 70 || speed > 100);

}



void generate_people(std::vector<Person>& people, const Graph& graph, int numPeople) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    std::unordered_set<int> visitable_nodes;
    for (int i = 0; i <= graph.nodes.size(); i++){
        visitable_nodes.insert(i);
    }

    for (int i = 0; i < numPeople; ++i) {
        int home_node_id = std::rand() % graph.nodes.size();
        int work_node_id = std::rand() % graph.nodes.size();
        people.emplace_back(home_node_id, work_node_id);
        people.back().dwalk=graph.dijkstra(home_node_id , visitable_nodes);
    }
}