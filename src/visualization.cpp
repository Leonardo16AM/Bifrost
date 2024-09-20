#include "visualization.h"

#ifdef _WIN32
void set_window_title_bar_color(sf::RenderWindow &window, COLORREF color)
{
    HWND hwnd = window.getSystemHandle();
    DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &color, sizeof(color));
}
#endif

void find_min_max_lat_lon(const Graph &graph, float &minLat, float &maxLat, float &minLon, float &maxLon)
{
    minLat = std::numeric_limits<float>::max();
    maxLat = std::numeric_limits<float>::lowest();
    minLon = std::numeric_limits<float>::max();
    maxLon = std::numeric_limits<float>::lowest();

    for (const auto &node : graph.nodes)
    {
        auto lat = node.lat;
        auto lon = node.lon;

        if (lat < minLat)
            minLat = lat;
        if (lat > maxLat)
            maxLat = lat;
        if (lon < minLon)
            minLon = lon;
        if (lon > maxLon)
            maxLon = lon;
    }
}

sf::Vector2f normalize_coordinates(float lat, float lon, float minLat, float maxLat, float minLon, float maxLon, float width, float height)
{
    float x = (lon - minLon) / (maxLon - minLon) * width;
    float y = (lat - minLat) / (maxLat - minLat) * height;
    return sf::Vector2f(x, height - y);
}

void precompute_normalized_coordinates(const Graph &graph, std::vector<NormalizedNode> &normalizedNodes, float minLat, float maxLat, float minLon, float maxLon, float width, float height)
{
    for (const auto &node : graph.nodes)
    {
        sf::Vector2f pos = normalize_coordinates(node.lat, node.lon, minLat, maxLat, minLon, maxLon, width, height);
        normalizedNodes.emplace_back(pos.x, pos.y);
    }
}

void draw_graph(sf::RenderWindow &window, const Graph &graph, const std::vector<NormalizedNode> &normalizedNodes)
{
    sf::VertexArray lines(sf::Quads, graph.edges.size() * 4);
    sf::VertexArray borders(sf::Quads, graph.edges.size() * 4);
    sf::VertexArray nodes(sf::Quads, normalizedNodes.size() * 4);

    float borderThickness = 0.7f;                  // Grosor del borde
    float lineThickness = borderThickness * 2 / 3; // Grosor de la línea original
    float nodeSize = 0.1f;
    float circleRadius = lineThickness / 2; // Radio del círculo debajo del nodo
    sf::CircleShape circle(circleRadius);
    circle.setFillColor(sf::Color(67, 86, 108)); // Color de la línea original

    size_t i = 0, j = 0;
    for (const auto &edge : graph.edges)
    {
        int source = edge.source;
        int target = edge.target;
        const sf::Vector2f &sourcePos = normalizedNodes[source].position;
        const sf::Vector2f &targetPos = normalizedNodes[target].position;

        sf::Vector2f direction = targetPos - sourcePos;
        float magnitude = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        sf::Vector2f unitDirection = direction / magnitude;
        sf::Vector2f perpendicular(-unitDirection.y, unitDirection.x);

        sf::Vector2f borderOffset = perpendicular * borderThickness / 2.0f;
        sf::Vector2f lineOffset = perpendicular * lineThickness / 2.0f;

        // Añadir bordes y líneas
        borders[i] = sf::Vertex(sourcePos - borderOffset, sf::Color(117, 126, 138));
        borders[i + 1] = sf::Vertex(sourcePos + borderOffset, sf::Color(117, 126, 138));
        borders[i + 2] = sf::Vertex(targetPos + borderOffset, sf::Color(117, 126, 138));
        borders[i + 3] = sf::Vertex(targetPos - borderOffset, sf::Color(117, 126, 138));

        lines[i] = sf::Vertex(sourcePos - lineOffset, sf::Color(67, 86, 108));
        lines[i + 1] = sf::Vertex(sourcePos + lineOffset, sf::Color(67, 86, 108));
        lines[i + 2] = sf::Vertex(targetPos + lineOffset, sf::Color(67, 86, 108));
        lines[i + 3] = sf::Vertex(targetPos - lineOffset, sf::Color(67, 86, 108));

        i += 4;
    }

    // for (const auto& node : normalizedNodes) {
    //     sf::Vector2f pos = node.position;
    //     circle.setPosition(pos - sf::Vector2f(circleRadius, circleRadius));
    //     window.draw(circle);

    //     nodes[j] = sf::Vertex(pos + sf::Vector2f(-nodeSize / 2, -nodeSize / 2), sf::Color(117, 126, 138));
    //     nodes[j+1] = sf::Vertex(pos + sf::Vector2f(nodeSize / 2, -nodeSize / 2), sf::Color(117, 126, 138));
    //     nodes[j+2] = sf::Vertex(pos + sf::Vector2f(nodeSize / 2, nodeSize / 2), sf::Color(117, 126, 138));
    //     nodes[j+3] = sf::Vertex(pos + sf::Vector2f(-nodeSize / 2, nodeSize / 2), sf::Color(117, 126, 138));

    //     j += 4;
    // }

    window.draw(borders);
    window.draw(lines);
    window.draw(nodes);

    // Dibujar un circulo rojo sobre los nodos 4811 4197 16859
    //  sf::CircleShape circle1(5);
    //  circle1.setPosition(normalizedNodes[4811].position);
    //  circle1.setFillColor(sf::Color::Red);
    //  window.draw(circle1);

    // sf::CircleShape circle2(5);
    // circle2.setPosition(normalizedNodes[4197].position);
    // circle2.setFillColor(sf::Color::Green);
    // window.draw(circle2);

    // sf::CircleShape circle3(5);
    // circle3.setPosition(normalizedNodes[16859].position);
    // circle3.setFillColor(sf::Color::Blue);
    // window.draw(circle3);

    // draw_people(window, graph.people, normalizedNodes, graph.edges);
}

void draw_people(sf::RenderWindow &window, const std::vector<Person> &people, const std::vector<NormalizedNode> &normalizedNodes, const std::vector<Edge> &edges)
{

    std::mt19937 gen(123);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    for (const auto &person : people)
    {
        sf::Vector2f homePos = normalizedNodes[person.home_node_id].position;
        sf::Vector2f workPos = normalizedNodes[person.work_node_id].position;

        int red = std::rand() % 256;
        int green = std::rand() % 256;
        int blue = std::rand() % 256;
        auto color = sf::Color(red, green, blue);

        int home_circle_radius = 1;
        sf::CircleShape circle(home_circle_radius);
        circle.setPosition(homePos - sf::Vector2f(home_circle_radius, home_circle_radius)); // Centrar el círculo
        circle.setFillColor(color);
        window.draw(circle);

        // auto Reccolor = generate_light_color();
        int work_square_size = 2;
        sf::RectangleShape square(sf::Vector2f(work_square_size, work_square_size));
        square.setPosition(workPos - sf::Vector2f(work_square_size / 2, work_square_size / 2)); // Centrar el cuadrado
        square.setFillColor(color);
        window.draw(square);
    }
}

float euclidean_distance(const sf::Vector2f &a, const sf::Vector2f &b)
{
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

void draw_text_with_outline(sf::RenderWindow &window, sf::Text &text, sf::Color outlineColor, float thickness)
{
    sf::Text outline = text;
    outline.setFillColor(outlineColor);
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if (dx != 0 || dy != 0)
            {
                outline.setPosition(text.getPosition().x + dx * thickness, text.getPosition().y + dy * thickness);
                window.draw(outline);
            }
        }
    }
    window.draw(text);
}

void draw_routes(sf::RenderWindow &window, const std::vector<Route> &routes, const std::vector<NormalizedNode> &normalizedNodes)
{
    std::map<std::string, sf::Color> route_colors;
    std::mt19937 gen(123);
    std::uniform_int_distribution<> dis(0, 255);

    for (const auto &route : routes)
    {
        const auto &nodes = route.nodes;

        if (route_colors.find(route.id) == route_colors.end())
        {
            route_colors[route.id] = sf::Color(dis(gen), dis(gen), dis(gen));
        }

        sf::Color routeColor = route_colors[route.id];

        float lineWidth = 0.5f; // Ancho de la línea

        for (size_t i = 0; i < nodes.size() - 1; ++i)
        {
            sf::Vector2f startPos = normalizedNodes[nodes[i]].position;
            sf::Vector2f endPos = normalizedNodes[nodes[i + 1]].position;

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

    for (const auto &route : routes)
    {
        for (const auto &stop : route.stops)
        {
            sf::Vector2f stopPos = normalizedNodes[stop].position;

            sf::CircleShape circle(0.5f);
            circle.setPosition(stopPos - sf::Vector2f(0.25f, 0.25f)); // Centrar el círculo
            circle.setFillColor(sf::Color::White);

            window.draw(circle);
        }
    }
}

void display_route_properties(sf::RenderWindow &window, const Route &route, const sf::Font &font, float windowWidth)
{
    sf::Text routeText;
    routeText.setFont(font);
    routeText.setCharacterSize(18);
    routeText.setFillColor(sf::Color::White);
    routeText.setPosition(windowWidth - 240, 800);

    routeText.setString("Route ID: " + route.id + "\n" +
                        "Distance: " + std::to_string(route.total_distance) + " km\n" +
                        "Buses: " + std::to_string(route.bus_count));

    draw_text_with_outline(window, routeText, sf::Color::Black);
}

sf::Color generate_light_color()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    int red = 128 + std::rand() % 128;
    int green = 128 + std::rand() % 128;
    int blue = 128 + std::rand() % 128;

    return sf::Color(red, green, blue);
}

void draw_partitioned_nodes(sf::RenderWindow &window, const std::map<int, int> &node_partition, const std::vector<NormalizedNode> &normalizedNodes)
{
    std::map<int, sf::Color> partition_colors;
    std::mt19937 gen(123);
    std::uniform_int_distribution<> dis(0, 255);

    for (const auto &np : node_partition)
    {
        if (partition_colors.find(np.second) == partition_colors.end())
        {
            partition_colors[np.second] = sf::Color(std::min(255, dis(gen) + 150), std::min(255, dis(gen) + 150), std::min(255, dis(gen) + 150));
        }
    }

    for (const auto &np : node_partition)
    {
        sf::CircleShape circle(0.5f);
        circle.setPosition(normalizedNodes[np.first].position);
        circle.setFillColor(partition_colors[np.second]);
        window.draw(circle);
    }
}

void draw_mincut(sf::RenderWindow &window, const std::vector<Edge> &mincut_edges, const std::vector<NormalizedNode> &normalizedNodes)
{
    sf::VertexArray lines(sf::Lines);

    for (const auto &edge : mincut_edges)
    {
        sf::Vector2f point1 = normalizedNodes[edge.source].position;
        sf::Vector2f point2 = normalizedNodes[edge.target].position;

        sf::Vertex vertex1(point1, sf::Color::Red);
        sf::Vertex vertex2(point2, sf::Color::Red);
        lines.append(vertex1);
        lines.append(vertex2);
    }

    window.draw(lines);
}