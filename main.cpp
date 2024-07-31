#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <filesystem>
#include "src/graph.h"
#include <SFML/Graphics.hpp>
#include <limits>
#include <cmath> // Para std::sqrt y std::pow

// Estructura para almacenar coordenadas normalizadas
struct NormalizedNode {
    sf::Vector2f position;
    NormalizedNode(float x, float y) : position(x, y) {}
};

// Función para encontrar el rango de latitudes y longitudes
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

// Función para normalizar las coordenadas
sf::Vector2f normalizeCoordinates(float lat, float lon, float minLat, float maxLat, float minLon, float maxLon, float width, float height) {
    float x = (lon - minLon) / (maxLon - minLon) * width;
    float y = (lat - minLat) / (maxLat - minLat) * height;
    return sf::Vector2f(x, height - y); // Invertir y para que el mapa no esté al revés
}

// Función para pre-calcular coordenadas normalizadas
void precomputeNormalizedCoordinates(const Graph& graph, std::vector<NormalizedNode>& normalizedNodes, float minLat, float maxLat, float minLon, float maxLon, float width, float height) {
    for (const auto& node : graph.nodes) {
        sf::Vector2f pos = normalizeCoordinates(std::stof(node.lat), std::stof(node.lon), minLat, maxLat, minLon, maxLon, width, height);
        normalizedNodes.emplace_back(pos.x, pos.y);
    }
}

void drawGraph(sf::RenderWindow& window, const Graph& graph, const std::vector<NormalizedNode>& normalizedNodes) {
    sf::VertexArray lines(sf::Lines);
    sf::VertexArray nodes(sf::Quads);

    for (const auto& edge : graph.edges) {
        int source = edge.source;
        int target = edge.target;

        lines.append(sf::Vertex(normalizedNodes[source].position));
        lines.append(sf::Vertex(normalizedNodes[target].position));
    }

    float nodeSize = 0.1f; // Tamaño del nodo

    for (const auto& node : normalizedNodes) {
        sf::Vector2f pos = node.position;

        nodes.append(sf::Vertex(pos + sf::Vector2f(-nodeSize / 2, -nodeSize / 2), sf::Color::Red));
        nodes.append(sf::Vertex(pos + sf::Vector2f(nodeSize / 2, -nodeSize / 2), sf::Color::Red));
        nodes.append(sf::Vertex(pos + sf::Vector2f(nodeSize / 2, nodeSize / 2), sf::Color::Red));
        nodes.append(sf::Vertex(pos + sf::Vector2f(-nodeSize / 2, nodeSize / 2), sf::Color::Red));
    }

    window.draw(lines);
    window.draw(nodes);
}


// Función para calcular la distancia euclidiana
float euclideanDistance(const sf::Vector2f& a, const sf::Vector2f& b) {
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

void drawTextWithOutline(sf::RenderWindow& window, sf::Text& text, sf::Color outlineColor, float thickness = 2.f) {
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

int main() {
    sf::RenderWindow window(sf::VideoMode(1600, 900), "Bifrost - Interactive Simulation");

    std::string nodes_file = "D:/Universidad/Inteligencia Artificial/Bifrost/maps/la_habana_nodes.csv"; 
    std::string edges_file = "D:/Universidad/Inteligencia Artificial/Bifrost/maps/la_habana_edges.csv"; 
    Graph graph = build_map(nodes_file, edges_file);

    float minLat, maxLat, minLon, maxLon;
    findMinMaxLatLon(graph, minLat, maxLat, minLon, maxLon);

    std::vector<NormalizedNode> normalizedNodes;
    precomputeNormalizedCoordinates(graph, normalizedNodes, minLat, maxLat, minLon, maxLon, window.getSize().x - 200, window.getSize().y); // Dejar espacio para la barra de logs

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
    logBar.setFillColor(sf::Color(70, 70, 75));
    logBar.setPosition(window.getSize().x - 250, 0);

    
    sf::Text logText;
    logText.setFont(font);
    logText.setCharacterSize(18);
    logText.setFillColor(sf::Color::White);
    logText.setPosition(window.getSize().x - 240, 10);

    bool dragging = false;
    sf::Vector2i oldMousePos;
    int selectedNodeId = -1;

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
                    if (euclideanDistance(worldPos, normalizedNodes[i].position) < 0.3) { // Aumentar el radio de selección
                        selectedNodeId = i;
                        break;
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
                    view.move(0, -10); // Move up
                } else if (event.key.code == sf::Keyboard::S) {
                    view.move(0, 10); // Move down
                } else if (event.key.code == sf::Keyboard::A) {
                    view.move(-10, 0); // Move left
                } else if (event.key.code == sf::Keyboard::D) {
                    view.move(10, 0); // Move right
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
                          "People: 50000\n" +
                          "Day: 7\n" +
                          "Hour: 9:41\n" +
                          "Avg Walked: 1.32 km\n" +
                          "Avg Travel Time: 45.5 min");
       
        window.clear(sf::Color(50, 50, 55));
        drawGraph(window, graph, normalizedNodes);

        // Restablecer la vista a la vista por defecto antes de dibujar el texto
        window.setView(window.getDefaultView());
        drawTextWithOutline(window, fpsText, sf::Color::Black);
        drawTextWithOutline(window, nodeIdText, sf::Color::Black);

        // Dibujar la barra de logs y el texto de los logs
        window.draw(logBar);
        window.draw(logText);

        // Volver a la vista del mapa
        window.setView(view);
        window.display();
    }

    return 0;
}
