#ifndef _BANKING_H_
#define _BANKING_H_

#include <vector>
#include "Cluster.h"
#include "Manager.h"
#include "Util.h"
#include "FF.h"

namespace bg = boost::geometry;

// 2-D point with coordinate type of double in cartesian
typedef bg::model::point<double, 2, bg::cs::cartesian> Point;

// Define a Point with an ID:
typedef std::pair<Point, int> PointWithID;

class Manager;
class Cluster;
class FF;

class Banking{
private:
    Manager& mgr;
    std::vector<FF *> FFs;
    std::vector<Cluster> clusters;
    std::unordered_map<int, int> clusterNum;

public:
    explicit Banking(Manager& mgr);
    ~Banking();

    void run();
    void libScoring();
    void sortCell(std::vector<Cell *> &cell_vector);
    Cell* chooseCandidateFF(FF* nowFF, Cluster& c, std::vector<PointWithID>& resultFFs, std::vector<PointWithID>& toRemoveFFs, std::vector<FF*> &FFToBank);
    Cell* chooseCellLib(int bitNum);
    Coor getMedian(std::vector<PointWithID>& toRemoveFFs);
    void sortFFs(std::vector<std::pair<int, double>> &nearFFs);
    void doClustering();
    void restoreUnclusterFFCoor();
    void ClusterResult();

};

#endif