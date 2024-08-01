#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <vector>

struct Position {
    int node_id; // -1 if on an edge
    int edge_id; // -1 if on a node
    float edge_position; // Position on the edge, if applicable
};

class Person {
public:
    Person(const std::string& name_, int home_node_id_, int work_node_id_);

    void move(float seconds);

    std::string name;
    int home_node_id;
    int work_node_id;
    Position current_position;
    int destination;
    std::vector<int> path;
};

void move_people(std::vector<Person>& people, float seconds);

#endif // PERSON_H
