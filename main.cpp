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
    sf::VertexArray nodes(sf::Points);

    for (const auto& edge : graph.edges) {
        int source = edge.source;
        int target = edge.target;

        lines.append(sf::Vertex(normalizedNodes[source].position));
        lines.append(sf::Vertex(normalizedNodes[target].position));
    }

    for (const auto& node : normalizedNodes) {
        nodes.append(sf::Vertex(node.position, sf::Color::Red));
    }

    window.draw(lines);
    window.draw(nodes);
}

int main() {
    sf::RenderWindow window(sf::VideoMode(1600, 900), "Mapa Interactivo");

    std::string nodes_file = "D:/Universidad/Inteligencia Artificial/Bifrost/maps/la_habana_nodes.csv"; 
    std::string edges_file = "D:/Universidad/Inteligencia Artificial/Bifrost/maps/la_habana_edges.csv"; 
    Graph graph = build_map(nodes_file, edges_file);

    float minLat, maxLat, minLon, maxLon;
    findMinMaxLatLon(graph, minLat, maxLat, minLon, maxLon);

    std::vector<NormalizedNode> normalizedNodes;
    precomputeNormalizedCoordinates(graph, normalizedNodes, minLat, maxLat, minLon, maxLon, window.getSize().x, window.getSize().y);

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

    bool dragging = false;
    sf::Vector2i oldMousePos;

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

        window.clear(sf::Color::Black);
        drawGraph(window, graph, normalizedNodes);

        // Restablecer la vista a la vista por defecto antes de dibujar el texto
        window.setView(window.getDefaultView());
        window.draw(fpsText);

        // Volver a la vista del mapa
        window.setView(view);
        window.display();
    }

    return 0;
}
