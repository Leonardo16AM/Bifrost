#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <limits>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include <SFML/Graphics.hpp>

#include "src/graph.h"
#include "src/route.h"
#include "src/person.h"
#include "src/visualization.h"
#include "src/inertial_flow.h"



int main() {
    std::string nodes_file = "D:/Universidad/Inteligencia Artificial/Bifrost/maps/la_habana_nodes.csv"; 
    std::string edges_file = "D:/Universidad/Inteligencia Artificial/Bifrost/maps/la_habana_edges.csv"; 
    printf("BUILDING MAP GRAPH\n");
    Graph graph = build_map(nodes_file, edges_file);
    
    std::vector<std::pair<int, int>> node_partition = inertial_flow_partition(graph);
    std::cout<<"CALCULATED INHERTIAL FLOW\n";



    std::vector<Route> routes = {};
    std::vector<Person> people;
    generate_people(people, graph, 10); 
    
    for(int i=0;i<people.size();i++){
        std::vector<int> shortest_path = graph.a_star(people[i].home_node_id,people[i].work_node_id);
        Route sp(std::to_string(i),  shortest_path,  shortest_path, 3, 2.343);
        routes.push_back(sp);
        printf("%d\n",shortest_path.size());
        if(i%100==0)std::cout<<i<<"\n";
    }
    


    sf::RenderWindow window(sf::VideoMode(1600, 900), "Bifrost - Interactive Simulation");

    sf::Image icon;
    if (!icon.loadFromFile("assets/icon.png")) {return -1;}
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    set_window_title_bar_color(window, RGB(20, 20, 26));
    


    float minLat, maxLat, minLon, maxLon;
    find_min_max_lat_lon(graph, minLat, maxLat, minLon, maxLon);

    std::vector<NormalizedNode> normalizedNodes;
    precompute_normalized_coordinates(graph, normalizedNodes, minLat, maxLat, minLon, maxLon, window.getSize().x - 200, window.getSize().y);

    sf::View view = window.getDefaultView();
    sf::Clock clock;
    
    sf::Font font;
    if (!font.loadFromFile("D:\\Universidad\\Inteligencia Artificial\\Bifrost\\assets\\SF-Pro.ttf")) {return -1;}
    
    sf::Text fpsText;
    fpsText.setFont(font);
    fpsText.setCharacterSize(20);
    fpsText.setFillColor(sf::Color::White);
    fpsText.setPosition(10, 10);

    sf::Text nodeIdText;
    nodeIdText.setFont(font);
    nodeIdText.setCharacterSize(20);
    nodeIdText.setFillColor(sf::Color::White);
    nodeIdText.setPosition(10, window.getSize().y - 30);

    sf::Text logText;
    logText.setFont(font);
    logText.setCharacterSize(20);
    logText.setFillColor(sf::Color::White);
    logText.setPosition(window.getSize().x - 250, 10);

    sf::Text routeInfoText;
    routeInfoText.setFont(font);
    routeInfoText.setCharacterSize(20);
    routeInfoText.setFillColor(sf::Color::White);
    routeInfoText.setPosition(window.getSize().x - 250, window.getSize().y - 100);

    bool dragging = false;
    sf::Vector2i oldMousePos;
    int selectedNodeId = -1;
    int selectedRouteId = -1;


    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.delta > 0) {
                    view.zoom(0.9f); // Zoom in
                } else {
                    view.zoom(1.1f); // Zoom out
                }
            }
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                dragging = true;
                oldMousePos = sf::Mouse::getPosition(window);

                sf::Vector2f worldPos = window.mapPixelToCoords(oldMousePos);
                for (size_t i = 0; i < normalizedNodes.size(); ++i) {
                    if (euclidean_distance(worldPos, normalizedNodes[i].position) < 0.3) {
                        selectedNodeId = i;
                        break;
                    }
                }

                for (size_t i = 0; i < routes.size(); ++i) {
                    for (const auto& stop : routes[i].stops) {
                        if (euclidean_distance(worldPos, normalizedNodes[stop].position) < 0.5) {
                            selectedRouteId = i;
                            break;
                        }
                    }
                }
            }
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                dragging = false;
            }
            if (event.type == sf::Event::MouseMoved) {
                if (dragging) {
                    sf::Vector2i newMousePos = sf::Mouse::getPosition(window);
                    sf::Vector2f delta = window.mapPixelToCoords(oldMousePos) - window.mapPixelToCoords(newMousePos);
                    view.move(delta);
                    oldMousePos = newMousePos;
                }
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::W) {
                    view.move(0, -5);
                } else if (event.key.code == sf::Keyboard::S) {
                    view.move(0, 5); 
                } else if (event.key.code == sf::Keyboard::A) {
                    view.move(-5, 0);
                } else if (event.key.code == sf::Keyboard::D) {
                    view.move(5, 0); 
                }
            }
        }

        window.setView(view);
        window.clear(sf::Color(20, 20, 26));
        
        float fps = 1.f / clock.restart().asSeconds();
        fpsText.setString("FPS: " + std::to_string(static_cast<int>(fps)));

        if (selectedNodeId != -1) {
            nodeIdText.setString("Node ID: " + std::to_string(selectedNodeId));
        }

        logText.setString("Nodes: " + std::to_string(graph.nodes.size()) + "\n" +
                          "Edges: " + std::to_string(graph.edges.size()) + "\n" +
                          "People: " + std::to_string(people.size()) + "\n" +
                          "Day: 7\n" +
                          "Hour: 9:41\n" +
                          "Avg Walked: 1.32 km\n" +
                          "Avg Travel Time: 45.5 min");

        draw_graph(window, graph, normalizedNodes);
        draw_partitioned_nodes(window, node_partition, normalizedNodes); // Asumiendo que tienes las coordenadas normalizadas        
        draw_routes(window, routes, normalizedNodes);
        // draw_people(window, people, normalizedNodes,graph.edges);


        window.setView(window.getDefaultView());
        draw_text_with_outline(window, fpsText, sf::Color::Black);
        draw_text_with_outline(window, nodeIdText, sf::Color::Black);
        draw_text_with_outline(window, logText, sf::Color::Black);

        if (selectedRouteId != -1) {
            routeInfoText.setString("Route ID: " + routes[selectedRouteId].id + "\n" +
                                    "Distance: " + std::to_string(routes[selectedRouteId].total_distance) + " km\n" +
                                    "Buses: " + std::to_string(routes[selectedRouteId].bus_count));
            draw_text_with_outline(window, routeInfoText, sf::Color::Black);
        }

        window.setView(view);
        window.display();
    }

    return 0;
}
