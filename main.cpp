#include "src/bifrost.h"

#define fl '\n'
#define fs first
#define sc second
using namespace std;

int BUSES = 100;
int ROUTES = 1;
int PERSONS = 1;

// Función para validar y extraer los datos
bool parseResponse(const string &response, string &archivo, int &personas, int &rutas, int &iteraciones, int &particulas)
{
    regex rgx(R"(.*?([^,\s]+\.csv)\s*,\s*personas:\s*(\d*)\s*,\s*rutas:\s*(\d*)\s*,\s*iteraciones:\s*(\d*)\s*,\s*particulas:\s*(\d*)\.?$)");
    smatch match;

    if (regex_search(response, match, rgx))
    {
        archivo = match[1].str();
        personas = stoi(match[2].str());
        rutas = stoi(match[3].str());
        iteraciones = stoi(match[4].str());
        particulas = stoi(match[5].str());
        return true;
    }
    return false;
}

int main(){

    LLMClient llm;
    string archivo = "nombre.csv";
    int personas = 10;
    int rutas = 10;
    int iteraciones = 5;
    int particulas = 30;  

    try
    {
        string user_input;
        cout << "Introduce el texto del usuario: ";
        getline(cin, user_input);
        bool can=false;
        int tries=0;       
        while(!can && tries<5){
            tries++;
            string response = llm.getResponse(user_input);

            if (!response.empty())
            {
                cout << "Respuesta del LLM: " << response << endl;
                if (parseResponse(response, archivo, personas, rutas, iteraciones, particulas))
                {
                    cout << "==Datos extraidos con exito:\n";
                    cout << "  Archivo: " << archivo << "\n";
                    cout << "  Personas: " << personas << "\n";
                    cout << "  Rutas: " << rutas << "\n";
                    cout << "  Iteraciones: " << iteraciones << "\n";
                    cout << "  Partículas: " << particulas << "\n";
                    can=true;
                }else{
                    cout << "No se pudo parsear la respuesta correctamente. Intenta nuevamente." << endl;
                }
                
                ROUTES = rutas;
                PERSONS = personas;
            }else{
                cout << "No se obtuvo una respuesta del LLM." << endl;
            }
        }
    }
    catch (const exception &e){
        cerr << "Error: " << e.what() << '\n';
    }

    string nodes_file = "./maps/la_habana_nodes.csv";
    string edges_file = "./maps/la_habana_edges.csv";

    printf("BUILDING MAP GRAPH\n");
    Graph graph = build_map(nodes_file, edges_file);
    // graph=graph.to_bidirectional();
    cout << "CALCULATING INERTIAL FLOW\n";
    map<int, int> node_partition = inertial_flow_partition_map(graph);

    cout << "CREATING TEST ROUTES" << endl;

    /*-------------------------- SIMULATION STUF --------------------------------------------------*/
    cout << "STARTING OPTIMIZATION" << endl;


    simulation S,best_sim;
    vector<Route> routes;
    vector<Person> people;
    
    
    if("nombre.csv"==archivo){
        auto start = chrono::high_resolution_clock::now();
        generate_people(people, graph, PERSONS);
        best_sim = Optimize(graph, people, ROUTES,iteraciones,particulas); 
        routes = best_sim.get_routes();
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = end - start;
        cout << "Training Time: " << elapsed.count() << " seconds" << endl;
    }else{
        cout << "LOADING FROM FILE" << endl;
        S.load_simulation_from_csv(archivo);
        routes = S.get_routes();
        people = S.get_people();
        simulation best_sim(routes, graph, people);
        best_sim.save_simulation_to_csv("test.csv");
    }
    
    double BEST_RESULTS;
    
    // {
    //     cout << "STARTING TEST SIMULATIONS" << endl;
    //     BEST_RESULTS = best_sim.simulate();
    //     cout << "RESULTS: " << BEST_RESULTS << endl;
    // }
    
    
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
