#include "GPTree.h"
#include <vector>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <algorithm>

using namespace std;

#define min(a, b) ((a) > (b) ? (b) : (a))

// permutation tables
const int permuteTable1[1][1] = {
    {0}
};
const int permuteTable2[2][2] = {
    {0, 1},
    {1, 0}
};
const int permuteTable3[6][3] = {
    {0, 1, 2}, {0, 2, 1},
    {1, 0, 2}, {1, 2, 0},
    {2, 0, 1}, {2, 1, 0}
};
const int permuteTable4[24][4] = {
    {0, 1, 2, 3}, {0, 1, 3, 2}, {0, 2, 1, 3}, {0, 2, 3, 1}, {0, 3, 1, 2}, {0, 3, 2, 1},
    {1, 0, 2, 3}, {1, 0, 3, 2}, {1, 2, 0, 3}, {1, 2, 3, 0}, {1, 3, 0, 2}, {1, 3, 2, 0},
    {2, 0, 1, 3}, {2, 0, 3, 1}, {2, 1, 0, 3}, {2, 1, 3, 0}, {2, 3, 0, 1}, {2, 3, 1, 0},
    {3, 0, 1, 2}, {3, 0, 2, 1}, {3, 1, 0, 2}, {3, 1, 2, 0}, {3, 2, 0, 1}, {3, 2, 1, 0}
};

// weights on cost
const double distWeight = 1.0;      // weight on dist to current position (D2)
const double detourWeight0 = 0.4;   // weight on detour for other passengers (D2 + D3 - D1)
const double detourWeight1 = 0.6;   // weight on detour for this passenger (D3 - D4)

// passenger
typedef struct Passenger {
    double lg;
    double lt;
    int dest;
} Passenger;

// vehicle
typedef struct Car {
    int id;
    int np;
    double lg;
    double lt;
    int pos;

    vector<Passenger> pass;

    // travel order
    int order[6];

    // statistics
    int dist;       // distance to current position
    int detour0;    // D2 + D3 - D1
    int detour1;    // D3 - D4

    // constructor
    Car(): pass(0) {
        id = np = pos = 0;
        lg = lt = 0.0;
    }
} Car;

// car and it's cost
typedef struct CarItem {
    int id;
    double cost;
} CarItem;

// coordinates
typedef struct Coords {
    int id;
    double lng;
    double lat;
} Coords;

// coordinate item
typedef struct CoordsItem {
    int id;
    double dist;
} CoordsItem;

// number of cars
int nCars = 0;

// cars
vector<Car> cars;

// available cars
vector<int> allCars;
vector<int> allCarPoses;

// all coordinates
vector<Coords> coords;

inline double costFunction(int dist, int detour0, int detour1) {
    return dist * distWeight + detour0 * detourWeight0 + detour1 * detourWeight1;
}

void read_cars() {
    const char *fileName = "../data/car.txt";
    FILE *fp = fopen(fileName, "r");

    int id, np;
    int pos;
    double lg, lt;
    while (fscanf(fp, "%d%d", &id, &np) == 2) {
        cars.emplace_back(Car());
        Car &car = cars.back();
        car.id = id;
        car.np = np;
        fscanf(fp, "%lf,%lf,%d", &car.lg, &car.lt, &car.pos);

        int pass_dest;
        double pass_lg, pass_lt;
        for (int pid = 0; pid < np; pid++) {
            fscanf(fp, "%lf,%lf,%d", &pass_lg, &pass_lt, &pass_dest);
            car.pass.emplace_back((Passenger){pass_lg, pass_lt, pass_dest});
        }
    }

    nCars = (int)cars.size();

    fclose(fp);
    fp = NULL;
}

void read_nodes() {
    const char *fileName = "../data/road.cnode";
    FILE *fp = fopen(fileName, "r");

    int nNodes = gptree_getsize();
    coords.resize(nNodes);

    int id;
    for (int i = 0; i < nNodes; i++)
        fscanf(fp, "%d%lf%lf", &coords[i].id, &coords[i].lng, &coords[i].lat);
    
    fclose(fp);
    fp = NULL;
}

double verify(int carId, int pos, int dest, int distToDest) { // distToDest = D4
    Car &car = cars[carId];
    car.order[0] = car.pos;
    car.order[1] = pos;
    
    int distToPos = gptree_search_catch(pos, car.pos);  // distToPos = D2
    // printf("D2: %d\n", distToPos);

    if (car.np == 0) {
        // printf("car %d: dist = %d, detour0 = 0, detour1 = 0\n", carId, distToPos);
        car.order[2] = dest;
        car.dist = distToPos;
        car.detour0 = 0;
        car.detour1 = 0;
        return (double)distToPos;
    }

    // initialize distances
    int dests[4];
    for (int i = 0; i < car.np; i++)
        dests[i] = car.pass[i].dest;
    dests[car.np] = dest;

    int distVec[4];
    for (int i = 0; i < car.np; i++)
        distVec[i] = gptree_search_catch(pos, dests[i]);
    distVec[car.np] = distToDest;

    int distVecSrc[4];
    for (int i = 0; i < car.np; i++)
        distVecSrc[i] = gptree_search_catch(car.pos, dests[i]);

    int distMat[4][4];
    for (int i = 0; i < car.np; i++)
        for (int j = i + 1; j <= car.np; j++)
            distMat[i][j] = distMat[j][i] = gptree_search_catch(dests[i], dests[j]);

    // permutation of all destinations (calculate D1 and D3)
    int D1 = 0x3fffffff, D3 = 0x3fffffff;
    int temp;
    int pBest = -1;
    switch (car.np) {
    case 1:
        D1 = distVecSrc[0];
        D3 = min(distVec[0], distVec[1]) + distMat[0][1];
        pBest = distVec[0] < distVec[1] ? 0 : 1;
        break;

    case 2:
        D1 = min(distVecSrc[0], distVecSrc[1]) + distMat[0][1];
        for (int pidx = 0; pidx < 6; pidx++) {
            temp = distVec[permuteTable3[pidx][0]];
            for (int i = 0; i < car.np; i++)
                temp += distMat[permuteTable3[pidx][i]][permuteTable3[pidx][i + 1]];
            if (temp < D3) {
                D3 = temp;
                pBest = pidx;
            }
        }
        break;

    case 3:
        for (int pidx = 0; pidx < 6; pidx++) {
            temp = distVecSrc[permuteTable3[pidx][0]];
            for (int i = 0; i < car.np; i++)
                temp += distMat[permuteTable3[pidx][i]][permuteTable3[pidx][i + 1]];
            D1 = temp < D1 ? temp : D1;
        }
        for (int pidx = 0; pidx < 24; pidx++) {
            temp = distVec[permuteTable4[pidx][0]];
            for (int i = 0; i < car.np; i++)
                temp += distMat[permuteTable4[pidx][i]][permuteTable4[pidx][i + 1]];
            if (temp < D3) {
                D3 = temp;
                pBest = pidx;
            }
        }
        break;

    default:
        printf("Error: invalid number of passengers\n");
    }

    // write orders
    switch (car.np) {
    case 1:
        for (int i = 0; i <= car.np; i++)
            car.order[i + 2] = dests[permuteTable2[pBest][i]];
        break;
    case 2:
        for (int i = 0; i <= car.np; i++)
            car.order[i + 2] = dests[permuteTable3[pBest][i]];
        break;
    case 3:
        for (int i = 0; i <= car.np; i++)
            car.order[i + 2] = dests[permuteTable4[pBest][i]];
        break;
    default:
        printf("Error: invalid number of passengers\n");
    }

    // print statistics
    int D2 = distToPos;
    int D4 = distToDest;
    int detour0 = D2 + D3 - D1;
    int detour1 = D3 - D4;

    car.dist = D2;
    car.detour0 = detour0;
    car.detour1 = detour1;

    if (detour0 <= 10000 && detour1 <= 10000) {
        // printf("car %d: dist = %d, detour0 = %d, detour1 = %d\n", carId, D2, detour0, detour1);
        return costFunction(D2, detour0, detour1);
    }
    else {
        return -1.0;
    }
}

// extract the content of stringstream
char *toString(ostringstream &oss) {
    const std::string &ostr = oss.str();
    char *retStr = new char[ostr.length() + 1];
    strcpy(retStr, ostr.c_str());

    return retStr;
}

extern "C" {

void init() {
    gptree_build();
    // printf("build finished\n");
    read_cars();
    read_nodes();

    // find available taxis
    allCars.clear();
    allCarPoses.clear();
    allCars.reserve(nCars);
    allCarPoses.reserve(nCars);

    int pos = 0;
    for (int i = 0; i < nCars; i++)
        if (cars[i].np < 4) {
            allCarPoses.push_back(cars[i].pos);
            allCars.push_back(i);
        }

    printf("Number of available cars: %d\n", (int)allCars.size());
}

char *query(int pos, int dest) {
    // find all taxis within distance
    vector<int> &&nearCars = gptree_KNN(pos, 100, allCarPoses);
    // vector<int> &&nearCars = gptree_KNN_bound(pos, 100, allCarPoses, 10000); // Bug
    // vector<int> &&nearCars = gptree_Range(pos, 10000, allCarPoses);
    // printf("Near cars: %d\n", (int)nearCars.size());

    // distance to destination
    int distToDest = gptree_search(pos, dest);
    // printf("D4: %d\n", distToDest);

    // verify each taxi
    vector<CarItem> resCars;
    resCars.reserve(100);

    for (int idx : nearCars) {
        int carId = allCars[idx];
        if (carId == 51354)
            printf("Correct!\n");
        double cost = verify(carId, pos, dest, distToDest);
        if (cost >= 0.0)
            resCars.emplace_back((CarItem){carId, cost}); 
    }
    sort(resCars.begin(), resCars.end(), [](CarItem &c1, CarItem &c2) {
        return c1.cost < c2.cost;
    });

    // save result
    printf("Result cars: %d\n", (int)resCars.size());
    // for (int i = 0; i < (int)resCars.size(); i++) {
    //     int carId = resCars[i].id;
    //     printf("car %d: cost = %.1lf, orders = ", carId, resCars[i].cost);
    //     for (int i = 0; i < cars[carId].np + 3; i++)
    //         printf("%d ", cars[carId].order[i]);
    //     printf("\n");
    // }

    vector<int> path;
    ostringstream oss;
    oss << "{\"result\":[";
    for (int i = 0; i < 5 && i < (int)resCars.size(); i++) {
        int carId = resCars[i].id;
        Car &car = cars[carId];

        oss << "[";
        oss << carId << "," << car.np + 3 << ",";
        oss << car.dist << "," << car.detour0 << ",";
        oss << car.detour1 << ",";

        oss << "[";
        for (int j = 0; j < car.np + 2; j++) {
            path.clear();
            gptree_find_path(car.order[j], car.order[j + 1], path);
            int pathLen = path.size();
            oss << "[";
            for (int idx = 0; idx < pathLen; idx++) {
                oss << "[" << path[idx] << ",";
                oss << coords[path[idx]].lng << ",";
                oss << coords[path[idx]].lat << "]";
                oss << (idx == pathLen - 1 ? "]" : ",");
            }
            oss << (j == car.np + 1 ? "]" : ",");
        }
        oss << "]";
        oss << (i == 4 ? "]" : ",");
    }
    oss << "}";
    // printf("%s\n", oss.str().c_str());

    return toString(oss);
}

char *position(double lng, double lat) {
    int nNodes = coords.size();

    vector<CoordsItem> coordsItems(coords.size());
    for (int i = 0; i < nNodes; i++) {
        double nlng = coords[i].lng;
        double nlat = coords[i].lat;
        coordsItems[i].id = i;
        coordsItems[i].dist = (lng - nlng) * (lng - nlng) + (lat - nlat) * (lat - nlat);
    }

    sort(coordsItems.begin(), coordsItems.end(), [](CoordsItem &c1, CoordsItem &c2) {
        return c1.dist < c2.dist;
    });

    ostringstream oss;
    oss << "{\"result\":[";
    if (coordsItems[0].dist < 0.00001) {
        int id = coordsItems[0].id;
        oss << id << "," << coords[id].lng << "," << coords[id].lat;
    }
    oss << "]}";

    return toString(oss);
}

void release(char *retStr) {
    if (retStr != NULL)
        delete[] retStr;
}

}
