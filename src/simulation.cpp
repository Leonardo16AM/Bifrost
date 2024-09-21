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
    fl::Engine* engine = new fl::Engine;
    engine->setName("DecisionMaking");

    fl::InputVariable* distance = new fl::InputVariable;
    distance->setName("Distance");
    distance->setRange(0.0, 10000.0);
    distance->addTerm(new fl::Triangle("Close", 0.0, 600.0, 1500.0));
    distance->addTerm(new fl::Triangle("Medium", 1200.0, 1500.0, 2500.0));
    distance->addTerm(new fl::Triangle("Far", 2300.0, 3500.0, 10000.0));
    engine->addInputVariable(distance);

    fl::InputVariable* busDelay = new fl::InputVariable;
    busDelay->setName("BusDelay");
    busDelay->setRange(0.0, 120.0);
    busDelay->addTerm(new fl::Triangle("Short", 0.0, 10.0, 20.0));
    busDelay->addTerm(new fl::Triangle("Medium", 15.0, 30.0, 45.0));
    busDelay->addTerm(new fl::Triangle("Long", 40.0, 60.0,120.0));
    engine->addInputVariable(busDelay);

    fl::InputVariable* physicalState = new fl::InputVariable;
    physicalState->setName("PhysicalState");
    physicalState->setRange(0.0, 1.0);
    physicalState->addTerm(new fl::Triangle("Weak", 0.0, 0.25, 0.5));
    physicalState->addTerm(new fl::Triangle("Average", 0.25, 0.5, 0.75));
    physicalState->addTerm(new fl::Triangle("Strong", 0.5, 0.75, 1.0));
    engine->addInputVariable(physicalState);

    fl::InputVariable* money = new fl::InputVariable;
    money->setName("Money");
    money->setRange(0.0, 1.0);
    money->addTerm(new fl::Triangle("Low", 0.0, 0.05, 0.1));
    money->addTerm(new fl::Triangle("Medium", 0.07, 0.4, 0.75));
    money->addTerm(new fl::Triangle("High", 0.5, 0.8, 1.0));
    engine->addInputVariable(money);

    fl::OutputVariable* decision = new fl::OutputVariable;
    decision->setName("Decision");
    decision->setRange(0.0, 1.0);
    decision->setDefaultValue(fl::nan);
    decision->setDefuzzifier(new fl::Centroid(100));
    decision->addTerm(new fl::Triangle("Walk", 0.0, 0.0, 0.5));
    decision->addTerm(new fl::Triangle("Bus", 0.5, 1.0, 1.0));
    decision->setAggregation(new fl::Maximum);
    engine->addOutputVariable(decision);

    fl::RuleBlock* rules = new fl::RuleBlock;
    rules->setName("Rules");
    rules->setConjunction(new fl::Minimum);
    rules->setDisjunction(new fl::Maximum);
    rules->setImplication(new fl::Minimum);
    rules->setActivation(new fl::General);

    rules->addRule(fl::Rule::parse("if Distance is Far and PhysicalState is Weak then Decision is Bus", engine));
    rules->addRule(fl::Rule::parse("if Money is Low then Decision is Walk", engine));
    rules->addRule(fl::Rule::parse("if BusDelay is Long then Decision is Walk", engine));
    rules->addRule(fl::Rule::parse("if PhysicalState is Strong and Distance is Close then Decision is Walk", engine));
    rules->addRule(fl::Rule::parse("if Distance is Medium and PhysicalState is Average then Decision is Walk", engine));
    rules->addRule(fl::Rule::parse("if Money is High then Decision is Bus", engine));
    engine->addRuleBlock(rules);

    std::unordered_map<int, std::pair<int, double>> um_walk = G.dijkstra(person.home_node_id, visitable_nodes);
    double walking_distance = um_walk[person.work_node_id].second; // en metros

    double delay = 10;

    distance->setValue(walking_distance);
    busDelay->setValue(delay);
    physicalState->setValue(person.phisical_state);
    money->setValue(person.money);

    std::string status;
    if (!engine->isReady(&status)) {
        throw fl::Exception("Engine not ready. The following errors were encountered:\n" + status, FL_AT);
    }

    engine->process();
    double decision_value = 0.7;
    decision_value=decision->getValue();

    if (decision_value < 0.5){
        long double sp = um_walk[person.work_node_id].second;
        delete engine;
        if(sp==std::numeric_limits<double>::infinity())sp=40.0;
        return sp / person.speed;
    }else{
        std::unordered_map<int, std::pair<int, double>> um = BG.dijkstra(person.home_node_id, visitable_nodes);
        long double sp = um[person.work_node_id].second;
        delete engine;
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