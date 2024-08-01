#include "person.h"
#include <cmath>
#include <iostream>

Person::Person(const std::string& name_, int home_node_id_, int work_node_id_)
    : name(name_), home_node_id(home_node_id_), work_node_id(work_node_id_), destination(-1) {
    current_position = {home_node_id_, -1, 0.0f};
}


void Person::move(float seconds) {
    // Implementation of moving logic
    if (destination == -1 || path.empty()) {
        return;
    }
    // Move logic for nodes and edges
    if (current_position.node_id != -1) {
        // Currently at a node
        if (!path.empty()) {
            current_position.node_id = path.front();
            path.erase(path.begin());
            if (path.empty()) {
                destination = -1; // Reached destination
            }
        }
    } else {
        // Currently on an edge
        current_position.edge_position += seconds; // Simulating movement along the edge
        if (current_position.edge_position >= 1.0f) {
            current_position.node_id = path.front();
            current_position.edge_id = -1;
            current_position.edge_position = 0.0f;
            path.erase(path.begin());
            if (path.empty()) {
                destination = -1; // Reached destination
            }
        }
    }
}

void move_people_kernel(Person* people, int num_people, float seconds) {
    
}

void move_people(std::vector<Person>& people, float seconds) {

}
