#include "person.h"
#include <cmath>
#include <iostream>
#include <random>


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

// int Person::choose_place_based_on_distance(const std::vector<double>& distances) {
//     std::vector<double> probabilities;
//     double total_prob = 0.0;

//     // Calcular la probabilidad para cada lugar usando una distribución exponencial
//     for (double d : distances) {
//         double prob = this->exp_lambda * exp(-this->exp_lambda * d);
//         probabilities.push_back(prob);
//         total_prob += prob;
//     }

//     // Normalizar las probabilidades
//     for (double& prob : probabilities) {
//         prob /= total_prob;
//     }

//     // Seleccionar un lugar basado en la probabilidad acumulada
//     std::random_device rd;
//     std::mt19937 gen(rd());
//     std::discrete_distribution<> dist(probabilities.begin(), probabilities.end());

//     return dist(gen);  // Retorna el índice del lugar elegido
// }


void generate_people(std::vector<Person>& people, const Graph& graph, int numPeople) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    for (int i = 0; i < numPeople; ++i) {
        int home_node_id = std::rand() % graph.nodes.size();
        int work_node_id = std::rand() % graph.nodes.size();
        people.emplace_back(home_node_id, work_node_id);
    }
}