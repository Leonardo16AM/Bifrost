#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <random>
#include <algorithm>
#include "graph.h"

struct Position {
    int node_id; // -1 if on an edge
    int edge_id; // -1 if on a node
    float edge_position; // Position on the edge, if applicable
};

class Person {
public:
    Person(int home_node_id_, int work_node_id_);

    int choose_place_based_on_distance(const std::vector<double>& distances);
    
    int home_node_id;
    int work_node_id;

    // Position current_position;
    // int destination;
    // std::vector<int> path;

    double exp_lambda=0.5;
    double norm_prob=0.7;
};

void generate_people(std::vector<Person>& people, const Graph& graph, int numPeople);

#endif // PERSON_H
