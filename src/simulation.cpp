#include "simulation.h"
#include "unordered_map"
#include <random>
#include <fl/Headers.h>
using namespace std;


simulation::simulation(std::vector<Route>buses_,Graph G_,int habitants=100):G(G_),buses(buses_){

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);


    generate_people(persons, G, habitants); 
    
    vector<Node>BN=G.nodes;
    vector<Edge>BE=G.edges;
    
    for(auto &e: BE){
        e.length=e.length/84.0;
    }

    int idn=BN.size();

    for(auto b:buses){
        double route_time=5*b.nodes.size();
        for(int i=0;i<b.stops.size();i++){
            
            Node node_wr=BN[b.stops[i]];
            BN.push_back(node_wr);

            Edge ne;
            ne.source=b.stops[i];
            ne.target=idn;
            ne.oneway=true;
            ne.length=(route_time* dis(gen) )/(double)b.bus_count;
            BE.push_back(ne);
            
            ne.source=idn;
            ne.target=b.stops[i];
            ne.oneway=true;
            ne.length=0;
            BE.push_back(ne);

            if(i==0)continue;

            
            ne.source=idn-1;
            ne.target=idn;
            ne.oneway=false;
            ne.length=1;

            BE.push_back(ne);
            
            idn++;
        }
    }

    BG=Graph(BN,BE);
}

double simulation::recursive_sim(std::vector<Person>&subset_pers,std::unordered_set<int> &visitable_nodes ){
    if(subset_pers.size()==1){
        Person person = subset_pers[0];

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
        double decision_value = decision->getValue();
        
        if (decision_value < 0.5) {
            long double sp = um_walk[person.work_node_id].second;
            delete engine;
            if(sp==std::numeric_limits<double>::infinity())sp=40.0;
            return sp / person.speed;
        } else {
            std::unordered_map<int, std::pair<int, double>> um = BG.dijkstra(person.home_node_id, visitable_nodes);
            long double sp = um[person.work_node_id].second;
            delete engine;
            if(sp==std::numeric_limits<double>::infinity())sp=40.0;
            return sp;
        }
    }


    vector<Person>L,R;
    for(int i=0;i<subset_pers.size()/2;i++){
        L.push_back(subset_pers[i]);
    }
    for(int i=subset_pers.size()/2;i<subset_pers.size();i++){
        R.push_back(subset_pers[i]);
    }
    double dl=recursive_sim(L,visitable_nodes );
    double dr=recursive_sim(R,visitable_nodes );
    return (double)(dl+dr)/2.0;
}

double simulation::simulate(int days){
    unsigned long long sum=0;
    int num_trips=0;

    std::unordered_set<int> visitable_nodes;
    for(int i=0;i<=BG.nodes.size();i++){
        visitable_nodes.insert(i);
    }

    return recursive_sim(persons,visitable_nodes );
}

