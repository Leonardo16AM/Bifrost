#ifndef FUZZYSIN_H
#define FUZZYSIN_H

#include "bifrost.h"
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include <string>
#include <stdexcept>

class FuzzyEngineSingleton {
public:
    static fl::Engine* getInstance() {
        static fl::Engine* engine = initializeEngine();
        return engine;
    }

private:
    static fl::Engine* initializeEngine() {
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
        busDelay->addTerm(new fl::Triangle("Long", 40.0, 60.0, 120.0));
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
    
        std::string status;
        if (!engine->isReady(&status)) {
            throw std::runtime_error("Fuzzy engine is not ready: " + status);
        }
    
        return engine;
    }
};

#endif