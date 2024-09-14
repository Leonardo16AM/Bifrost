#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <iostream>
#include <vector>
#include <random>
#include <set>
#include <utility>
#include <cmath>
#include <limits>

#include "graph.h"
#include "person.h"
#include "simulation.h"
#include "route.h"

using namespace std;

int pointToNode(double lat, double lon, Graph& graph);
vector<Route> Optimize(Graph& graph, int population, int number_of_routes);

#endif 