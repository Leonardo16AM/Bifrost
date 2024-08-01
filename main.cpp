#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <filesystem>
#include <SFML/Graphics.hpp>
#include <limits>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "src/graph.h"
#include "src/route.h"
#include "src/person.h" 

#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

void setWindowTitleBarColor(sf::RenderWindow& window, COLORREF color) {
    HWND hwnd = window.getSystemHandle();
    
    // Establece el color de la barra de título para Windows 10 y posteriores
    DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &color, sizeof(color));
}

// Estructura para almacenar coordenadas normalizadas
struct NormalizedNode {
    sf::Vector2f position;
    NormalizedNode(float x, float y) : position(x, y) {}
};

void findMinMaxLatLon(const Graph& graph, float& minLat, float& maxLat, float& minLon, float& maxLon);
sf::Vector2f normalizeCoordinates(float lat, float lon, float minLat, float maxLat, float minLon, float maxLon, float width, float height);
void precomputeNormalizedCoordinates(const Graph& graph, std::vector<NormalizedNode>& normalizedNodes, float minLat, float maxLat, float minLon, float maxLon, float width, float height);
void drawGraph(sf::RenderWindow& window, const Graph& graph, const std::vector<NormalizedNode>& normalizedNodes);
void drawPeople(sf::RenderWindow& window, const std::vector<Person>& people, const std::vector<NormalizedNode>& normalizedNodes);
float euclideanDistance(const sf::Vector2f& a, const sf::Vector2f& b);
void drawTextWithOutline(sf::RenderWindow& window, sf::Text& text, sf::Color outlineColor, float thickness = 2.f);

void drawRoutes(sf::RenderWindow& window, const std::vector<Route>& routes, const std::vector<NormalizedNode>& normalizedNodes);
void displayRouteProperties(sf::RenderWindow& window, const Route& route, const sf::Font& font, float windowWidth);


void generatePeople(std::vector<Person>& people, const Graph& graph, int numPeople);
int main() {
    sf::RenderWindow window(sf::VideoMode(1600, 900), "Bifrost - Interactive Simulation");

    sf::Image icon;
    if (!icon.loadFromFile("assets/icon.png")) {
        return -1; // Manejar error de carga del icono
    }
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

    // Cambiar el color de la barra de título
    setWindowTitleBarColor(window, RGB(20, 20, 26));

    std::string nodes_file = "D:/Universidad/Inteligencia Artificial/Bifrost/maps/la_habana_nodes.csv"; 
    std::string edges_file = "D:/Universidad/Inteligencia Artificial/Bifrost/maps/la_habana_edges.csv"; 
    Graph graph = build_map(nodes_file, edges_file);

    std::vector<Route> routes = {
    };

    std::vector<int> stops = {375, 21249, 1205, 1214, 1226, 1235, 16905, 1165, 1145, 1123, 1117, 1098, 1084, 1073, 17136};
    Route p5("P5", stops, stops, 3, 323);
    routes.push_back(p5);

    float minLat, maxLat, minLon, maxLon;
    findMinMaxLatLon(graph, minLat, maxLat, minLon, maxLon);

    std::vector<NormalizedNode> normalizedNodes;
    precomputeNormalizedCoordinates(graph, normalizedNodes, minLat, maxLat, minLon, maxLon, window.getSize().x - 200, window.getSize().y);

    sf::View view = window.getDefaultView();
    sf::Clock clock;
    sf::Font font;
    if (!font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
        return -1; // Manejar error de carga de fuente
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

    sf::RectangleShape logBar(sf::Vector2f(250, window.getSize().y));
    logBar.setFillColor(sf::Color(20, 20, 26));
    logBar.setPosition(window.getSize().x - 250, 0);

    sf::Text logText;
    logText.setFont(font);
    logText.setCharacterSize(18);
    logText.setFillColor(sf::Color::White);
    logText.setPosition(window.getSize().x - 240, 10);

    bool dragging = false;
    sf::Vector2i oldMousePos;
    int selectedNodeId = -1;
    int selectedRouteId = -1;

    std::vector<Person> people;
    generatePeople(people, graph, 10000); // Generar 10,000 personas

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
                    if (euclideanDistance(worldPos, normalizedNodes[i].position) < 0.3) {
                        selectedNodeId = i;
                        break;
                    }
                }

                for (size_t i = 0; i < routes.size(); ++i) {
                    for (const auto& stop : routes[i].stops) {
                        if (euclideanDistance(worldPos, normalizedNodes[stop].position) < 0.5) {
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
                    view.move(0, -5); // Move up
                } else if (event.key.code == sf::Keyboard::S) {
                    view.move(0, 5); // Move down
                } else if (event.key.code == sf::Keyboard::A) {
                    view.move(-5, 0); // Move left
                } else if (event.key.code == sf::Keyboard::D) {
                    view.move(5, 0); // Move right
                }
            }
        }

        window.setView(view);

        float fps = 1.f / clock.restart().asSeconds();
        fpsText.setString("FPS: " + std::to_string(static_cast<int>(fps)));

        if (selectedNodeId != -1) {
            nodeIdText.setString("Node ID: " + std::to_string(selectedNodeId));
        }

        // Actualizar el texto de los logs
        logText.setString("Nodes: " + std::to_string(graph.nodes.size()) + "\n" +
                          "Edges: " + std::to_string(graph.edges.size()) + "\n" +
                          "People: " + std::to_string(people.size()) + "\n" +
                          "Day: 7\n" +
                          "Hour: 9:41\n" +
                          "Avg Walked: 1.32 km\n" +
                          "Avg Travel Time: 45.5 min");

        window.clear(sf::Color(26, 38, 54));
        drawGraph(window, graph, normalizedNodes);
        drawRoutes(window, routes, normalizedNodes);
        drawPeople(window, people, normalizedNodes);

        for (auto& person : people) {
            person.move(1.0f); // Mover a cada persona (ajusta el tiempo según sea necesario)
        }

        window.setView(window.getDefaultView());
        drawTextWithOutline(window, fpsText, sf::Color::Black);
        drawTextWithOutline(window, nodeIdText, sf::Color::Black);

        window.draw(logBar);
        window.draw(logText);

        if (selectedRouteId != -1) {
            displayRouteProperties(window, routes[selectedRouteId], font, window.getSize().x);
        }

        window.setView(view);
        window.display();
    }

    return 0;
}


void findMinMaxLatLon(const Graph& graph, float& minLat, float& maxLat, float& minLon, float& maxLon) {
    minLat = std::numeric_limits<float>::max();
    maxLat = std::numeric_limits<float>::lowest();
    minLon = std::numeric_limits<float>::max();
    maxLon = std::numeric_limits<float>::lowest();

    for (const auto& node : graph.nodes) {
        float lat = std::stof(node.lat);
        float lon = std::stof(node.lon);

        if (lat < minLat) minLat = lat;
        if (lat > maxLat) maxLat = lat;
        if (lon < minLon) minLon = lon;
        if (lon > maxLon) maxLon = lon;
    }
}

sf::Vector2f normalizeCoordinates(float lat, float lon, float minLat, float maxLat, float minLon, float maxLon, float width, float height) {
    float x = (lon - minLon) / (maxLon - minLon) * width;
    float y = (lat - minLat) / (maxLat - minLat) * height;
    return sf::Vector2f(x, height - y);
}

void precomputeNormalizedCoordinates(const Graph& graph, std::vector<NormalizedNode>& normalizedNodes, float minLat, float maxLat, float minLon, float maxLon, float width, float height) {
    for (const auto& node : graph.nodes) {
        sf::Vector2f pos = normalizeCoordinates(std::stof(node.lat), std::stof(node.lon), minLat, maxLat, minLon, maxLon, width, height);
        normalizedNodes.emplace_back(pos.x, pos.y);
    }
}

void drawGraph(sf::RenderWindow& window, const Graph& graph, const std::vector<NormalizedNode>& normalizedNodes) {
    sf::VertexArray lines(sf::Quads);
    sf::VertexArray borders(sf::Quads);
    sf::VertexArray nodes(sf::Quads);

    float borderThickness = 1.5f; // Grosor del borde
    float lineThickness = borderThickness * 2 / 3; // Grosor de la línea original

    for (const auto& edge : graph.edges) {
        int source = edge.source;
        int target = edge.target;

        sf::Vector2f sourcePos = normalizedNodes[source].position;
        sf::Vector2f targetPos = normalizedNodes[target].position;

        sf::Vector2f direction = targetPos - sourcePos;
        sf::Vector2f unitDirection = direction / std::sqrt(direction.x * direction.x + direction.y * direction.y);
        sf::Vector2f perpendicular(-unitDirection.y, unitDirection.x);

        sf::Vector2f borderOffset = perpendicular * borderThickness / 2.0f;
        sf::Vector2f lineOffset = perpendicular * lineThickness / 2.0f;

        // Añadir borde negro
        borders.append(sf::Vertex(sourcePos - borderOffset, sf::Color::Black));
        borders.append(sf::Vertex(sourcePos + borderOffset, sf::Color::Black));
        borders.append(sf::Vertex(targetPos + borderOffset, sf::Color::Black));
        borders.append(sf::Vertex(targetPos - borderOffset, sf::Color::Black));

        // Añadir línea original
        lines.append(sf::Vertex(sourcePos - lineOffset, sf::Color(67, 86, 108)));
        lines.append(sf::Vertex(sourcePos + lineOffset, sf::Color(67, 86, 108)));
        lines.append(sf::Vertex(targetPos + lineOffset, sf::Color(67, 86, 108)));
        lines.append(sf::Vertex(targetPos - lineOffset, sf::Color(67, 86, 108)));
    }

    float nodeSize = 0.1f;

    for (const auto& node : normalizedNodes) {
        sf::Vector2f pos = node.position;

        nodes.append(sf::Vertex(pos + sf::Vector2f(-nodeSize / 2, -nodeSize / 2), sf::Color::Red));
        nodes.append(sf::Vertex(pos + sf::Vector2f(nodeSize / 2, -nodeSize / 2), sf::Color::Red));
        nodes.append(sf::Vertex(pos + sf::Vector2f(nodeSize / 2, nodeSize / 2), sf::Color::Red));
        nodes.append(sf::Vertex(pos + sf::Vector2f(-nodeSize / 2, nodeSize / 2), sf::Color::Red));
    }

    // Dibujar primero los bordes
    window.draw(borders);
    // Dibujar las líneas originales
    window.draw(lines);
    // Dibujar los nodos
    window.draw(nodes);
}



void drawPeople(sf::RenderWindow& window, const std::vector<Person>& people, const std::vector<NormalizedNode>& normalizedNodes) {
    sf::VertexArray personQuads(sf::Quads);

    float personSize = 0.2f; // Tamaño del cuadro que representa a una persona

    for (const auto& person : people) {
        sf::Vector2f pos = normalizedNodes[person.current_position.node_id].position;

        personQuads.append(sf::Vertex(pos + sf::Vector2f(-personSize / 2, -personSize / 2), sf::Color::Blue));
        personQuads.append(sf::Vertex(pos + sf::Vector2f(personSize / 2, -personSize / 2), sf::Color::Blue));
        personQuads.append(sf::Vertex(pos + sf::Vector2f(personSize / 2, personSize / 2), sf::Color::Blue));
        personQuads.append(sf::Vertex(pos + sf::Vector2f(-personSize / 2, personSize / 2), sf::Color::Blue));
    }

    window.draw(personQuads);
}

float euclideanDistance(const sf::Vector2f& a, const sf::Vector2f& b) {
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

void drawTextWithOutline(sf::RenderWindow& window, sf::Text& text, sf::Color outlineColor, float thickness) {
    sf::Text outline = text;
    outline.setFillColor(outlineColor);
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx != 0 || dy != 0) {
                outline.setPosition(text.getPosition().x + dx * thickness, text.getPosition().y + dy * thickness);
                window.draw(outline);
            }
        }
    }
    window.draw(text);
}

void drawRoutes(sf::RenderWindow& window, const std::vector<Route>& routes, const std::vector<NormalizedNode>& normalizedNodes) {
    for (const auto& route : routes) {
        const auto& stops = route.stops;
        sf::Color routeColor = route.color;
        float lineWidth = 0.5f; // Ancho de la línea

        for (size_t i = 0; i < stops.size() - 1; ++i) {
            sf::Vector2f startPos = normalizedNodes[stops[i]].position;
            sf::Vector2f endPos = normalizedNodes[stops[i + 1]].position;

            sf::Vector2f direction = endPos - startPos;
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            direction /= length;

            sf::RectangleShape line(sf::Vector2f(length, lineWidth));
            line.setPosition(startPos);
            line.setFillColor(routeColor);
            line.setRotation(std::atan2(direction.y, direction.x) * 180.f / 3.14159265f);

            window.draw(line);
        }
    }
}

void displayRouteProperties(sf::RenderWindow& window, const Route& route, const sf::Font& font, float windowWidth) {
    sf::Text routeText;
    routeText.setFont(font);
    routeText.setCharacterSize(18);
    routeText.setFillColor(sf::Color::White);
    routeText.setPosition(windowWidth - 240, 800);

    routeText.setString("Route ID: " + route.id + "\n" +
                        "Distance: " + std::to_string(route.total_distance) + " km\n" +
                        "Buses: " + std::to_string(route.bus_count));

    drawTextWithOutline(window, routeText, sf::Color::Black);
}

void generatePeople(std::vector<Person>& people, const Graph& graph, int numPeople) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    for (int i = 0; i < numPeople; ++i) {
        int home_node_id = std::rand() % graph.nodes.size();
        int work_node_id = std::rand() % graph.nodes.size();
        std::string name = "Person_" + std::to_string(i);

        people.emplace_back(name, home_node_id, work_node_id);
    }
}
