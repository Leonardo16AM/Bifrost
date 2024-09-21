#include "bifrost.h"
using namespace std;

simulation::simulation() {}

simulation::simulation(std::vector<Route> buses_, Graph G_, vector<Person> persons_) : G(G_), buses(buses_), persons(persons_){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    // generate_people(persons, G, habitants);

    vector<Node> BN = G.nodes;
    vector<Edge> BE = G.edges;

    for (auto &e : BE){
        e.length = e.length / 84.0;
    }

    int idn = BN.size();

    for (auto b : buses){
        double route_time = 5 * b.nodes.size();
        for (int i = 0; i < b.stops.size(); i++){

            Node node_wr = BN[b.stops[i]];
            BN.push_back(node_wr);

            Edge ne;
            ne.source = b.stops[i];
            ne.target = idn;
            ne.oneway = false;
            ne.length = (route_time * dis(gen)) / (double)b.bus_count;
            BE.push_back(ne);

            ne.source = idn;
            ne.target = b.stops[i];
            ne.oneway = true;
            ne.length = 0;
            BE.push_back(ne);

            if (i == 0)
                continue;

            ne.source = idn - 1;
            ne.target = idn;
            ne.oneway = false;
            ne.length = 1;

            BE.push_back(ne);

            idn++;
        }
    }

    BG = Graph(BN, BE);
}


double simulation::simulate_person(Person &person, std::unordered_set<int> &visitable_nodes){
    fl::Engine* engine = FuzzyEngineSingleton::getInstance();

    double walking_distance = person.dwalk[person.work_node_id].second; 

    double delay = 10.0; 

    engine->getInputVariable("Distance")->setValue(walking_distance);
    engine->getInputVariable("BusDelay")->setValue(delay);
    engine->getInputVariable("PhysicalState")->setValue(person.phisical_state);
    engine->getInputVariable("Money")->setValue(person.money);

    std::string status;
    if (!engine->isReady(&status)) {
        throw fl::Exception("Engine not ready. The following errors were encountered:\n" + status, FL_AT);
    }

    engine->process();
    double decision_value = engine->getOutputVariable("Decision")->getValue();

    if (decision_value < 0.5){
        double sp = person.dwalk[person.work_node_id].second;
        if(sp == std::numeric_limits<double>::infinity()) sp = 40.0;
        return sp / person.speed;
    } else {
        std::unordered_map<int, std::pair<int, double>> um = BG.dijkstra(person.home_node_id, visitable_nodes);
        double sp = um[person.work_node_id].second;
        if (sp == std::numeric_limits<double>::infinity())
            sp = 40.0;
        return sp;
    }
}

double simulation::average_sim(std::vector<Person> &subset_pers, std::unordered_set<int> &visitable_nodes){
    if (subset_pers.size() == 1)
        simulate_person(subset_pers[0],visitable_nodes);

    vector<Person> L, R;
    for (int i = 0; i < subset_pers.size() / 2; i++){
        L.push_back(subset_pers[i]);
    }
    for (int i = subset_pers.size() / 2; i < subset_pers.size(); i++){
        R.push_back(subset_pers[i]);
    }
    double dl = average_sim(L, visitable_nodes);
    double dr = average_sim(R, visitable_nodes);
    return (double)(dl + dr) / 2.0;
}

double simulation::percentil90_sim(std::vector<Person> &subset_pers, std::unordered_set<int> &visitable_nodes){
    vector<double>vals;
    for(auto& pers:subset_pers){
        vals.push_back(simulate_person(pers,visitable_nodes));
    }
    sort(vals.begin(),vals.end());

     if(vals.empty()){
        // Manejar el caso donde no hay valores
        return 0.0;
    }
    
    double pos = 0.9 * static_cast<double>(vals.size());
    size_t index = static_cast<size_t>(std::ceil(pos)) - 1; 
    
    if(index >= vals.size()){
        index = vals.size() - 1;
    }
    
    return vals[index];
}

double simulation::simulate(int days){
    unsigned long long sum = 0;
    int num_trips = 0;

    std::unordered_set<int> visitable_nodes;
    for (int i = 0; i <= BG.nodes.size(); i++){
        visitable_nodes.insert(i);
    }

    return percentil90_sim(persons, visitable_nodes);
}

std::vector<Route> simulation::get_routes(){
    return buses;
}

std::vector<Person> simulation::get_people(){
    return persons;
}

void simulation::save_simulation_to_csv(const std::string &filename) const{
    std::ofstream file(filename);
    if (!file.is_open()){
        std::cerr << "Error: Unable to open file " << filename << " for saving.\n";
        return;
    }

    // Guardar la información de los autobuses
    file << "RouteID,BusCount,TotalDistance,Color,Stops,Nodes\n";
    for (const auto &route : buses){
        file << route.id << "," << route.bus_count << ","
             << route.total_distance << ","
             << route.color.toInteger() << ",";
        // Guardar las paradas
        for (size_t i = 0; i < route.stops.size(); ++i){
            file << route.stops[i];
            if (i < route.stops.size() - 1)
                file << ";";
        }
        file << ",";

        // Guardar los nodos
        for (size_t i = 0; i < route.nodes.size(); ++i){
            file << route.nodes[i];
            if (i < route.nodes.size() - 1)
                file << ";";
        }
        file << "\n";
    }

    // Guardar la información de las personas
    file << "HomeNodeID,WorkNodeID,PhisicalState,Patience,Money,Speed\n";
    for (const auto &person : persons){
        file << person.home_node_id << ","
             << person.work_node_id << ","
             << person.phisical_state << ","
             << person.patience << ","
             << person.money << ","
             << person.speed << "\n";
    }

    file.close();
    std::cout << "Simulation data saved to " << filename << ".\n";
}

// Función para cargar la simulación desde un archivo CSV
void simulation::load_simulation_from_csv(const std::string &filename){
    std::ifstream file(filename);
    if (!file.is_open()){
        std::cerr << "Error: Unable to open file " << filename << " for loading.\n";
        return;
    }

    std::string line, token;

    // Cargar las rutas
    buses.clear();
    std::getline(file, line); // Ignorar encabezado
    while (std::getline(file, line)){
        if (line == "HomeNodeID,WorkNodeID,PhisicalState,Patience,Money,Speed")
            break; // Llegamos a la sección de personas
        std::istringstream ss(line);
        Route route;
        std::getline(ss, route.id, ',');
        std::getline(ss, token, ',');
        route.bus_count = std::stoi(token);
        std::getline(ss, token, ',');
        route.total_distance = std::stod(token);
        std::getline(ss, token, ',');
        route.color = sf::Color(std::stoul(token));

        // Cargar las paradas
        std::getline(ss, token, ',');
        std::istringstream stops_ss(token);
        std::string stop;
        while (std::getline(stops_ss, stop, ';')){
            route.stops.push_back(std::stoi(stop));
        }

        // Cargar los nodos
        std::getline(ss, token, ',');
        std::istringstream nodes_ss(token);
        std::string node;
        while (std::getline(nodes_ss, node, ';')){
            route.nodes.push_back(std::stoi(node));
        }

        buses.push_back(route);
    }

    // Cargar las personas
    persons.clear();
    while (std::getline(file, line)){
        std::istringstream ss(line);
        Person person;
        std::getline(ss, token, ',');
        person.home_node_id = std::stoi(token);
        std::getline(ss, token, ',');
        person.work_node_id = std::stoi(token);
        std::getline(ss, token, ',');
        person.phisical_state = std::stod(token);
        std::getline(ss, token, ',');
        person.patience = std::stod(token);
        std::getline(ss, token, ',');
        person.money = std::stod(token);
        std::getline(ss, token, ',');
        person.speed = std::stod(token);

        persons.push_back(person);
    }

    file.close();
    std::cout << "Simulation data loaded from " << filename << ".\n";
}