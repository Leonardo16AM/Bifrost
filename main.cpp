#include "src/bifrost.h"

#define fl '\n'
#define fs first
#define sc second
using namespace std;

int BUSES = 100;
int ROUTES = 3;
int PERSONS = 16;

int main()
{

    LLMClient llm;
    try
    {
        std::string response = llm.getResponse("How many Rs are in strawberry");
        if (!response.empty())
        {
            std::cout << "Respuesta del LLM: " << response << std::endl;
        }
        else
        {
            std::cout << "No se obtuvo una respuesta del LLM." << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
    }

    string nodes_file = "./maps/la_habana_nodes.csv";
    string edges_file = "./maps/la_habana_edges.csv";

    printf("BUILDING MAP GRAPH\n");
    Graph graph = build_map(nodes_file, edges_file);
    // graph=graph.to_bidirectional();
    cout << "CALCULATING INERTIAL FLOW\n";
    map<int, int> node_partition = inertial_flow_partition_map(graph);

    cout << "CREATING TEST ROUTES" << endl;

    // // Generamos 100 rutas random (fijate que las persons que usamos ahi no sirven para nada, es solo para generar los inicios y fiin de las rutas)
    // vector<Route> routes = {};
    // vector<Person> route_inits;
    // generate_people(route_inits, graph, ROUTES);

    // for (int i = 0; i < route_inits.size(); i++) {
    //     // //Creamos una ruta desde HOME hasta WORK
    //     // Route sp(graph, to_string(i), {route_inits[i].home_node_id,route_inits[i].work_node_id}, 10);
    //     // //Hacemos que cada 5 calles tengamos una parada
    //     // sp.stops.pop_back();
    //     // if(sp.nodes.size()==0)continue;
    //     // for(int i=0;i<sp.nodes.size()-1;i++){
    //     //     if(i%5==0){
    //     //         sp.stops.push_back(sp.nodes[i]);
    //     //     }
    //     // }
    //     // sp.stops.push_back(sp.nodes.back());

    //     Route sp = create_route(graph, to_string(i), route_inits[i].home_node_id, route_inits[i].work_node_id, 10);

    //     if(sp.nodes.size()>0)
    //         routes.push_back(sp);
    // }

    /*-------------------------- SIMULATION STUF --------------------------------------------------*/
    cout << "STARTING OPTIMIZATION" << endl;

    auto start = std::chrono::high_resolution_clock::now();
    vector<Person> people;
    generate_people(people, graph, PERSONS);
    auto best_sim = Optimize(graph, people, ROUTES); // persons, routes
    vector<Route> routes = best_sim.get_routes();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Training Time: " << elapsed.count() << " seconds" << std::endl;


    cout<< "LOADING FROM FILE" << endl;
    simulation S;
    S.load_simulation_from_csv("test.csv");
    vector<Route> routes = S.get_routes();
    vector<Person> people = S.get_people();
    simulation best_sim(routes, graph, people);

    double BEST_RESULTS;
    {
        cout << "STARTING TEST SIMULATIONS" << endl;
        auto start = std::chrono::high_resolution_clock::now();

        // simulation S(routes, graph, people);
        BEST_RESULTS = best_sim.simulate();
        cout << "RESULTS: " << BEST_RESULTS << endl;

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Time taken: " << elapsed.count() << " seconds" << std::endl;
    }

    // Saving and loading
    {
        cout << "SAVING" << endl;
        best_sim.save_simulation_to_csv("test.csv");
        
        // simulation S;
        // S.simulate();
    }

    /*-------------------------- VISUALIZER STUF --------------------------------------------------*/
    {
        sf::RenderWindow window(sf::VideoMode(1600, 900), "Bifrost - Interactive Simulation");

        sf::Image icon;
        if (!icon.loadFromFile("assets/icon.png"))
        {
            return -1;
        }
        window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

#ifdef _WIN32
        set_window_title_bar_color(window, RGB(20, 20, 26));
#endif

        float minLat, maxLat, minLon, maxLon;
        find_min_max_lat_lon(graph, minLat, maxLat, minLon, maxLon);

        vector<NormalizedNode> normalizedNodes;
        precompute_normalized_coordinates(graph, normalizedNodes, minLat, maxLat, minLon, maxLon, window.getSize().x - 200, window.getSize().y);

        sf::View view = window.getDefaultView();
        sf::Clock clock;

        sf::Font font;
        if (!font.loadFromFile("./assets/SF-Pro.ttf"))
        {
            return -1;
        }

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

        while (window.isOpen())
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                }
                if (event.type == sf::Event::MouseWheelScrolled)
                {
                    if (event.mouseWheelScroll.delta > 0)
                    {
                        view.zoom(0.9f); // Zoom in
                    }
                    else
                    {
                        view.zoom(1.1f); // Zoom out
                    }
                }
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                {
                    dragging = true;
                    oldMousePos = sf::Mouse::getPosition(window);

                    sf::Vector2f worldPos = window.mapPixelToCoords(oldMousePos);
                    for (size_t i = 0; i < normalizedNodes.size(); ++i)
                    {
                        if (euclidean_distance(worldPos, normalizedNodes[i].position) < 0.3)
                        {
                            selectedNodeId = i;
                            break;
                        }
                    }

                    for (size_t i = 0; i < routes.size(); ++i)
                    {
                        for (const auto &stop : routes[i].stops)
                        {
                            if (euclidean_distance(worldPos, normalizedNodes[stop].position) < 0.5)
                            {
                                selectedRouteId = i;
                                break;
                            }
                        }
                    }
                }
                if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
                {
                    dragging = false;
                }
                if (event.type == sf::Event::MouseMoved)
                {
                    if (dragging)
                    {
                        sf::Vector2i newMousePos = sf::Mouse::getPosition(window);
                        sf::Vector2f delta = window.mapPixelToCoords(oldMousePos) - window.mapPixelToCoords(newMousePos);
                        view.move(delta);
                        oldMousePos = newMousePos;
                    }
                }
                if (event.type == sf::Event::KeyPressed)
                {
                    if (event.key.code == sf::Keyboard::W)
                    {
                        view.move(0, -5);
                    }
                    else if (event.key.code == sf::Keyboard::S)
                    {
                        view.move(0, 5);
                    }
                    else if (event.key.code == sf::Keyboard::A)
                    {
                        view.move(-5, 0);
                    }
                    else if (event.key.code == sf::Keyboard::D)
                    {
                        view.move(5, 0);
                    }
                }
            }

            window.setView(view);
            window.clear(sf::Color(20, 20, 26));

            float fps = 1.f / clock.restart().asSeconds();
            fpsText.setString("FPS: " + to_string(static_cast<int>(fps)));

            if (selectedNodeId != -1)
            {
                nodeIdText.setString("Node ID: " + to_string(selectedNodeId));
            }

            logText.setString("Nodes: " + to_string(graph.nodes.size()) + "\n" +
                              "Edges: " + to_string(graph.edges.size()) + "\n" +
                              "People: " + to_string(PERSONS) + "\n" +
                              "Routes: " + to_string(ROUTES) + "\n" +
                              "Buses: " + to_string(BUSES) + "\n" +
                              "Avg Travel Time: " + to_string((int)BEST_RESULTS) + "min");

            draw_graph(window, graph, normalizedNodes);
            // draw_partitioned_nodes(window, node_partition, normalizedNodes); // Asumiendo que tienes las coordenadas normalizadas
            draw_routes(window, routes, normalizedNodes);
            draw_people(window, people, normalizedNodes, graph.edges);

            window.setView(window.getDefaultView());
            draw_text_with_outline(window, fpsText, sf::Color::Black);
            draw_text_with_outline(window, nodeIdText, sf::Color::Black);
            draw_text_with_outline(window, logText, sf::Color::Black);

            if (selectedRouteId != -1)
            {
                routeInfoText.setString("Route ID: " + routes[selectedRouteId].id + "\n" +
                                        "Distance: " + to_string(routes[selectedRouteId].total_distance) + " km\n" +
                                        "Buses: " + to_string(routes[selectedRouteId].bus_count));
                draw_text_with_outline(window, routeInfoText, sf::Color::Black);
            }

            window.setView(view);
            window.display();
        }
    }

    return 0;
}
