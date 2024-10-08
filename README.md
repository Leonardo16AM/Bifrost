# Bifrost: Agent-Based Public Transportation Optimization

Bifrost is a simulation project aimed at optimizing public transportation in an urban environment using a multi-agent system. The project leverages the Belief-Desire-Intention (BDI) architecture to model agents representing city travelers, and Particle Swarm Optimization (PSO) to minimize the Conditional Value at Risk at the 90th percentile (CVaR90) of travel times. The objective is to create an efficient, reliable public transport system by modeling real-world traveler behavior and optimizing the network.

![](assets/inertial.png)


## Features

- **Agent-Based Modeling**: Uses agents with beliefs, desires, and intentions to simulate the decision-making of public transit users.
- **Fuzzy Logic Integration**: Fuzzy rules are applied to help agents decide between different transportation modes (e.g., walk vs. bus).
- **Event-Based Simulation**: The simulation progresses based on real-world events, such as people starting to walk or buses arriving at stops.
- **Optimization via PSO**: The bus routes are optimized using Particle Swarm Optimization, with a focus on minimizing CVaR90 to ensure reliability in worst-case scenarios.
