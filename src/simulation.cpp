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

simulation::simulation(vector<Route> buses_, Graph G_, vector<Person> &persons_,vector<double>&cost_per_person, bool event_based) : G(G_), buses(buses_), persons(persons_){
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 1.0);

    vector<Node> BN = G.nodes;
    vector<Edge> BE = G.edges;

    for (auto &e : BE){
        e.length = e.length / 84.0;
    }

    int idn = BN.size();
    original_n=BN.size();

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

    if(event_based){
        unordered_set<int>visitable_nodes;
        for(int i=0;i<=BN.size();i++)visitable_nodes.insert(i);
        
        for(int i=0;i<persons.size();i++){
            if(persons[i].beliefs.size()==0)persons[i].beliefs=base_beliefs;
            sim_person p;
            p.id=0;

            vector<pair<int,double>> old_beliefs=BG.update_from_beliefs(persons[i].beliefs);
            auto dijkstra_result = BG.dijkstra(persons[i].home_node_id, visitable_nodes);
            vector<int> shortest_path = BG.reconstruct_path(persons[i].home_node_id, persons[i].work_node_id, dijkstra_result);

            double sp=dijkstra_result[persons[i].work_node_id].second;
            if (sp == numeric_limits<double>::infinity())
                    sp = 40.0;
            cost_per_person.push_back(sp);

            p.path_nodes=shortest_path;

            for(auto it:shortest_path){
                if(it>original_n)cout<<"<<< ";
                cout<<">>> >>>"<<it<<endl;
            }

            BG.update_from_beliefs(old_beliefs);    
            for(auto it:p.path_nodes){
                if(it>=original_n){
                    p.distances.push_back(1);
                }else{
                    p.distances.push_back(2);
                }
            }
            sim_persons.push_back(p);
            
            event person_walk = {0, event_type::PERSON_START_WALKING, i, p.path_nodes[0]};
            event_queue.push(person_walk);
        }

        for(int i=0;i<buses.size();i++){
            for(int j=0;j<buses[i].bus_count;j++){
                sim_bus b;
                b.route_id=i;
                b.id=sim_buses.size();
                b.stops=exp_nodes[i];
                b.distances=vector<double>(100);
                
                if(rand()%2==0)
                    reverse(b.stops.begin(), b.stops.end());
                
                b.current_stop = rand() % b.stops.size();
                for(int h=0;h<=b.current_stop;h++){
                    b.visited.insert(b.stops[h]);
                } 
                    
                event bus_arrival = {dis(gen), event_type::BUS_ARRIVE_STOP, b.id,b.stops[b.current_stop]};
                event_queue.push(bus_arrival);

                sim_buses.push_back(b);
            }
        }
    }
}

std::string event_type_to_string(event_type type) {
    switch (type) {
        case event_type::PERSON_START_WALKING:
            return "PERSON_START_WALKING";
        case event_type::PERSON_ARRIVE_STOP:
            return "PERSON_ARRIVE_STOP";
        case event_type::BUS_ARRIVE_STOP:
            return "BUS_ARRIVE_STOP";
        case event_type::PERSON_GET_OFF_BUS:
            return "PERSON_GET_OFF_BUS";
        case event_type::PERSON_ARRIVE_WORK:
            return "PERSON_ARRIVE_WORK";
        default:
            return "UNKNOWN";
    }
}


void simulation::handle_person_start_walking(const event& curr_event) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 1.0);

    sim_person* p=&sim_persons[curr_event.entity_id];
    double distance_walked=0.0;
    while(p->current_stop<p->path_nodes.size() && p->path_nodes[p->current_stop]<original_n){
        distance_walked+=p->move();
    }

    if(p->current_stop==p->path_nodes.size()){
        event arrive(
            distance_walked*persons[curr_event.entity_id].speed+(dis(gen)-0.5),
            event_type::PERSON_ARRIVE_WORK,
            p->id,
            p->path_nodes.back());

        event_queue.push(arrive);
    }else{
        event arrive(
            distance_walked*persons[curr_event.entity_id].speed+(dis(gen)-0.5) ,
            event_type::PERSON_ARRIVE_STOP, 
            p->id,
            p->path_nodes[p->current_stop]
        );

        event_queue.push(arrive);    
    }
}

void simulation::handle_person_arrive_stop(const event& curr_event) {
}

void simulation::handle_bus_arrive_stop(const event& curr_event) {
}

void simulation::handle_person_get_off_bus(const event& curr_event) {
}

void simulation::handle_person_arrive_work(const event& curr_event) {
}



double simulation::events_simulate(int days){
    random_device rd;
    mt19937 gen(rd()); 
    uniform_real_distribution<> dis(0.0, 1.0);

    while(!event_queue.empty()){
        event curr=event_queue.top();
        event_queue.pop();

        cout<<">>> "<<event_type_to_string(curr.type)<<" "<<curr.entity_id<<" "<<curr.node<<endl;
        
        switch(curr.type) {
            case event_type::PERSON_START_WALKING:
                handle_person_start_walking(curr);
                break;
            case event_type::PERSON_ARRIVE_STOP:
                handle_person_arrive_stop(curr);
                break;
            case event_type::BUS_ARRIVE_STOP:
                handle_bus_arrive_stop(curr);
                break;
            case event_type::PERSON_GET_OFF_BUS:
                handle_person_get_off_bus(curr);
                break;
            case event_type::PERSON_ARRIVE_WORK:
                handle_person_arrive_work(curr);
                break;
        }
    }
    return -1;
}


double simulation::simulate_person(Person &person, unordered_set<int> &visitable_nodes){
    fl::Engine* engine = FuzzyEngineSingleton::getInstance();

    double walking_distance = person.dwalk[person.work_node_id].second; 

    double delay = 10.0; 

    engine->getInputVariable("Distance")->setValue(walking_distance);
    engine->getInputVariable("BusDelay")->setValue(delay);
    engine->getInputVariable("PhysicalState")->setValue(person.phisical_state);
    engine->getInputVariable("Money")->setValue(person.money);

    string status;
    if (!engine->isReady(&status)) {
        throw fl::Exception("Engine not ready. The following errors were encountered:\n" + status, FL_AT);
    }

    engine->process();
    double decision_value = engine->getOutputVariable("Decision")->getValue();

    if (decision_value < 0.5){
        double sp = person.dwalk[person.work_node_id].second;
        if(sp == numeric_limits<double>::infinity()) sp = 40.0;
        return sp / person.speed;
    } else {
        vector<pair<int,double>> old_beliefs=BG.update_from_beliefs(person.beliefs);

        unordered_map<int, pair<int, double>> um = BG.dijkstra(person.home_node_id, visitable_nodes);
        double sp = um[person.work_node_id].second;
        if (sp == numeric_limits<double>::infinity())
            sp = 40.0;

        BG.update_from_beliefs(old_beliefs);    
        return sp;
    }
}

vector<double> simulation::simulate_persons(vector<Person> &subset_pers, unordered_set<int> &visitable_nodes){
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

double simulation::CVaR90(vector<Person> &subset_pers, unordered_set<int> &visitable_nodes){
    vector<double>vals=simulate_persons(subset_pers,visitable_nodes);
    sort(vals.begin(),vals.end());
    reverse(vals.begin(),vals.end());

     if(vals.empty()){
        return 0.0;
    }
    double pos = 0.9 * static_cast<double>(vals.size());
    size_t index = static_cast<size_t>(ceil(pos)) - 1; 
    
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

    unordered_set<int> visitable_nodes;
    for (int i = 0; i <= BG.nodes.size(); i++){
        visitable_nodes.insert(i);
    }

    return CVaR90(persons, visitable_nodes);
}

vector<Route> simulation::get_routes(){
    return buses;
}

vector<Person> simulation::get_people(){
    return persons;
}

void simulation::save_simulation_to_csv(const string &filename) const{
    ofstream file(filename);
    if (!file.is_open()){
        cerr << "Error: Unable to open file " << filename << " for saving.\n";
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
    cout << "Simulation data saved to " << filename << ".\n";
}

void simulation::load_simulation_from_csv(const string &filename){
    ifstream file(filename);
    if (!file.is_open()){
        cerr << "Error: Unable to open file " << filename << " for loading.\n";
        return;
    }

    string line, token;

    // Cargar las rutas
    buses.clear();
    getline(file, line); // Ignorar encabezado
    while (getline(file, line)){
        if (line == "HomeNodeID,WorkNodeID,PhisicalState,Patience,Money,Speed")
            break; // Llegamos a la sección de personas
        istringstream ss(line);
        Route route;
        getline(ss, route.id, ',');
        getline(ss, token, ',');
        route.bus_count = stoi(token);
        getline(ss, token, ',');
        route.total_distance = stod(token);
        getline(ss, token, ',');
        route.color = sf::Color(stoul(token));

        // Cargar las paradas
        getline(ss, token, ',');
        istringstream stops_ss(token);
        string stop;
        while (getline(stops_ss, stop, ';')){
            route.stops.push_back(stoi(stop));
        }

        // Cargar los nodos
        getline(ss, token, ',');
        istringstream nodes_ss(token);
        string node;
        while (getline(nodes_ss, node, ';')){
            route.nodes.push_back(stoi(node));
        }

        buses.push_back(route);
    }

    // Cargar las personas
    persons.clear();
    while (getline(file, line)){
        istringstream ss(line);
        Person person;
        getline(ss, token, ',');
        person.home_node_id = stoi(token);
        getline(ss, token, ',');
        person.work_node_id = stoi(token);
        getline(ss, token, ',');
        person.phisical_state = stod(token);
        getline(ss, token, ',');
        person.patience = stod(token);
        getline(ss, token, ',');
        person.money = stod(token);
        getline(ss, token, ',');
        person.speed = stod(token);

        persons.push_back(person);
    }

    file.close();
    cout << "Simulation data loaded from " << filename << ".\n";
}