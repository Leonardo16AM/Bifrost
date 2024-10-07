#include "bifrost.h"
using namespace std;

bool event::operator<(const event& other) const {
    return time > other.time;
}


double sim_person::move(){
    current_stop++;
    return distances[current_stop-1];
}

bool sim_person::on_work(){
    return current_stop==distances.size()-1;
}


double sim_bus::move(){
    if(current_stop==stops.size()-1){
        reverse(stops.begin(),stops.end());
        reverse(distances.begin(),distances.end());
        visited.clear();
        visited.insert(stops[0]);
        current_stop=1;
        return distances[0];
    }
    visited.insert(stops[current_stop]);
    current_stop++;
    return distances[current_stop-1];
}

vector<int> sim_bus::leave_on_stop(){
    vector<int>ret= passengers[stops[current_stop]];
    passengers[stops[current_stop]].clear();
    return ret;
}

bool sim_bus::on_direction(int node){
    return visited.find(node)!=visited.end();
}


simulation::simulation() {}

simulation::simulation(std::vector<Route> buses_, Graph G_, vector<Person> &persons_,vector<double>&cost_per_person) : G(G_), buses(buses_), persons(persons_){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    vector<Node> BN = G.nodes;
    vector<Edge> BE = G.edges;

    for (auto &e : BE){
        e.length = e.length / 84.0;
    }

    int idn = BN.size();
    int original_n=BN.size();

    vector<pair<int,double>>base_beliefs;
    vector<vector<int>>exp_nodes;

    for (auto b : buses){
        double route_time = 5 * b.nodes.size();
        vector<int>exp;
        for (int i = 0; i < b.stops.size(); i++){

            Node node_wr = BN[b.stops[i]];
            BN.push_back(node_wr);

            exp.push_back(idn);
            Edge ne;
            ne.source = b.stops[i];
            ne.target = idn;
            ne.oneway = false;
            ne.length = (route_time * dis(gen)) / (double)b.bus_count;
            base_beliefs.push_back({BE.size(),ne.length});
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
            base_beliefs.push_back({BE.size(),ne.length} );
            BE.push_back(ne);

            idn++;
        }
        exp_nodes.push_back(exp);
    }

    BG = Graph(BN, BE);


    unordered_set<int>visitable_nodes;
    for(int i=0;i<=BN.size();i++)visitable_nodes.insert(i);
    
    for(int i=0;i<persons.size();i++){
        if(persons[i].beliefs.size()==0)persons[i].beliefs=base_beliefs;
        sim_person p;
        p.id=0;

        std::vector<std::pair<int,double>> old_beliefs=BG.update_from_beliefs(persons[i].beliefs);
        auto dijkstra_result = BG.dijkstra(persons[i].home_node_id, visitable_nodes);
        vector<int> shortest_path = BG.reconstruct_path(persons[i].home_node_id, persons[i].work_node_id, dijkstra_result);

        double sp=dijkstra_result[persons[i].work_node_id].second;
        if (sp == std::numeric_limits<double>::infinity())
                sp = 40.0;
        cost_per_person.push_back(sp);

        p.path_nodes=shortest_path;
        for(auto it:shortest_path){
            if(it>original_n)cout<<"<<< ";
            cout<<">>> >>>"<<it<<endl;
        }
        BG.update_from_beliefs(old_beliefs);    
        // UPDATE p.distances;
        sim_persons.push_back(p);
    }

    for(int i=0;i<buses.size();i++){
        for(int j=0;j<buses[i].bus_count;j++){
            sim_bus b;
            b.route_id=i;
            b.id=sim_buses.size();
            b.stops=exp_nodes[i];
            b.distances=vector<double>(0.5);
            
            if(std::rand()%2==0)
                std::reverse(b.stops.begin(), b.stops.end());
            
            b.current_stop = std::rand() % b.stops.size();
            for(int h=0;h<=b.current_stop;h++){
                b.visited.insert(b.stops[h]);
            } 

            sim_buses.push_back(b);
        }
    }
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
        std::vector<std::pair<int,double>> old_beliefs=BG.update_from_beliefs(person.beliefs);

        std::unordered_map<int, std::pair<int, double>> um = BG.dijkstra(person.home_node_id, visitable_nodes);
        double sp = um[person.work_node_id].second;
        if (sp == std::numeric_limits<double>::infinity())
            sp = 40.0;

        BG.update_from_beliefs(old_beliefs);    
        return sp;
    }
}

vector<double> simulation::simulate_persons(std::vector<Person> &subset_pers, std::unordered_set<int> &visitable_nodes){
    vector<double>vals;
    for(auto& pers:subset_pers){
        vals.push_back(simulate_person(pers,visitable_nodes));
    }
    return vals;
}

double simulation::average(vector<double> &vals){
    if (vals.size() == 1)
        return vals[0];

    vector<double> L, R;
    for (int i = 0; i < vals.size() / 2; i++){
        L.push_back(vals[i]);
    }
    for (int i = vals.size() / 2; i < vals.size(); i++){
        R.push_back(vals[i]);
    }
    double dl = average(L);
    double dr = average(R);
    return (double)(dl + dr) / 2.0;
}

double simulation::CVaR90(std::vector<Person> &subset_pers, std::unordered_set<int> &visitable_nodes){
    vector<double>vals=simulate_persons(subset_pers,visitable_nodes);
    sort(vals.begin(),vals.end());
    reverse(vals.begin(),vals.end());

     if(vals.empty()){
        return 0.0;
    }
    double pos = 0.9 * static_cast<double>(vals.size());
    size_t index = static_cast<size_t>(std::ceil(pos)) - 1; 
    
    if(index >= vals.size()){
        index = vals.size() - 1;
    }
    
    int nindex=0;
    for(int i=0;i<20;i++){
        if( (1<<i)>=vals.size())break;

        if( (1<<i) >= index){
            nindex=(1<<i);
            break;
        }else{
            nindex=(1<<i);
        }
    }
    index=nindex;
    vector<double>to_avg;
    for(int i=0;i<=index;i++){
        to_avg.push_back(vals[i]);
    }
    return average(to_avg);
}

double simulation::simulate(int days){
    unsigned long long sum = 0;
    int num_trips = 0;

    std::unordered_set<int> visitable_nodes;
    for (int i = 0; i <= BG.nodes.size(); i++){
        visitable_nodes.insert(i);
    }

    return CVaR90(persons, visitable_nodes);
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