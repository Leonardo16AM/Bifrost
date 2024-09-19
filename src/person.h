#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <random>
#include <algorithm>
#include "graph.h"

struct Position
{
    int node_id;         // -1 if on an edge
    int edge_id;         // -1 if on a node
    float edge_position; // Position on the edge, if applicable
};

class Person
{
public:
    Person(int home_node_id_, int work_node_id_);
    Person();

    int home_node_id;
    int work_node_id;

    double phisical_state, patience, money, speed;
};

void generate_people(std::vector<Person> &people, const Graph &graph, int numPeople);

#endif // PERSON_H