#include "optimizer.h"


using namespace std;

set<pair<double, int>> lats, lons;


// Estructura para una celda de la cuadrícula
struct GridCell {
    std::vector<Node> nodes;  // Nodos en esta celda
};

class SpatialGrid {
public:
    SpatialGrid(double gridSize, const Graph& graph) : gridSize(gridSize) {
        // Llenamos la cuadrícula con los nodos del grafo
        for (const auto& node : graph.nodes) {
            double lat = node.lat;
            double lon = node.lon;
            auto cell = getCell(lat, lon);
            grid[cell].nodes.push_back(node);
        }
    }

    // Encuentra el nodo más cercano dado latitud y longitud
    int nearestNeighbor(double lat, double lon) {
        auto cell = getCell(lat, lon);
        double minDist = std::numeric_limits<double>::max();
        int closestNodeId = -1;

        // Verificamos la celda actual y las celdas vecinas
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                auto neighborCell = std::make_pair(cell.first + i, cell.second + j);
                if (grid.find(neighborCell) != grid.end()) {
                    for (const auto& node : grid[neighborCell].nodes) {
                        double nodeLat = node.lat;
                        double nodeLon = node.lon;
                        double dist = distance(lat, lon, nodeLat, nodeLon);
                        if (dist < minDist) {
                            minDist = dist;
                            closestNodeId = node.id;
                        }
                    }
                }
            }
        }

        return closestNodeId;
    }

private:
    double gridSize;
    std::map<std::pair<int, int>, GridCell> grid;

    // Función que convierte latitud y longitud a una celda de la cuadrícula
    std::pair<int, int> getCell(double lat, double lon) {
        int x = static_cast<int>(lat / gridSize);
        int y = static_cast<int>(lon / gridSize);
        return {x, y};
    }

    // Función para calcular la distancia euclidiana entre dos puntos
    double distance(double lat1, double lon1, double lat2, double lon2) {
        return std::sqrt((lat1 - lat2) * (lat1 - lat2) + (lon1 - lon2) * (lon1 - lon2));
    }
};

// Función que devuelve el id del nodo más cercano dada la latitud y longitud
int pointToNode(double lat, double lon, const Graph& graph) {
    // Definimos el tamaño de la cuadrícula (puede ajustarse según las características del grafo)
    static SpatialGrid spatialGrid(0.01, graph);
    return spatialGrid.nearestNeighbor(lat, lon);
}

// devuelve el id del nodo mas cercano a la latitud y longitud dada
int pointToNode(double lat, double lon, const Graph& graph, bool wtf){
    
    //revisar todos los nodos y devolver el qe este mas cerca
    double minDist = 1e9;
    int node = -1;

    for(int i=0;i<graph.nodes.size();i++){
        double dist = sqrt((graph.nodes[i].lat-lat)*(graph.nodes[i].lat-lat) + (graph.nodes[i].lon-lon)*(graph.nodes[i].lon-lon));
        if(dist<minDist){
            minDist = dist;
            node = i;
        }
    }

    return node;
}


void Optimize(const Graph& graph, const vector<Person>& people) {

    for(int i=0;i<graph.nodes.size();i++){
        lats.insert({graph.nodes[i].lat, graph.nodes[i].id});
        lons.insert({graph.nodes[i].lon, graph.nodes[i].id});
    }

    // imprimir la mayor y menor latitud y longitud
    cout<<"lats range: "<<lats.begin()->first<<" "<<lats.rbegin()->first<<"\nlon range:"<<lons.begin()->first<<" "<<lons.rbegin()->first<<endl;


    cout<<graph.nodes[3].lat<<" "<<graph.nodes[3].lon<<" NODO 3\n";
    int nod = pointToNode(22, -82, graph);
    cout<<"FOUND NODO "<<nod<<" "<<graph.nodes[nod].lat<<" "<<graph.nodes[nod].lon<<"\n";

}


