#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <iostream>
#include <vector>
#include <random>
#include <set>
#include <utility>

#include "graph.h"
#include "person.h"
#include "simulation.h"
#include "route.h"

using namespace std;

int pointToNode(double lat, double lon, Graph& graph);
void Optimize(const Graph& graph, const vector<Person>& people);

#endif 