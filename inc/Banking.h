#ifndef _BANKING_H_
#define _BANKING_H_

#ifdef ENABLE_DEBUG_BAN
#define DEBUG_BAN(message) std::cout << "[BANKING] " << message << std::endl
#else
#define DEBUG_BAN(message)
#endif

#include <vector>
#include "Cluster.h"
#include "Manager.h"
#include "Util.h"
#include "FF.h"
#include "Legalizer.h"

namespace bg = boost::geometry;

// 2-D point with coordinate type of double in cartesian
typedef bg::model::point<double, 2, bg::cs::cartesian> Point;

// Define a Point with an ID:
typedef std::pair<Point, int> PointWithID;

class Manager;
class Cluster;
class FF;
class Legalizer;

class Banking{
private:
    Manager& mgr;
    std::vector<FF *> FFs;
    std::unordered_map<int, int> clusterNum;
    std::vector<int> bitOrder;

public:
    explicit Banking(Manager& mgr);
    ~Banking();

    void run();

    void bitOrdering();
    bool chooseCandidateFF(FF* nowFF, std::vector<PointWithID>& resultFFs, std::vector<PointWithID>& toRemoveFFs, std::vector<FF*> &FFToBank, const int &targetBit);
    // Cell* chooseCellLib(int bitNum);
    Coor getMedian(std::vector<PointWithID>& toRemoveFFs);
    static void sortFFs(std::vector<std::pair<int, double>> &nearFFs);
    void doClustering();
    void restoreUnclusterFFCoor();
    void ClusterResult();
    double CostCompare(const Coor clusterCoor, Cell* chooseCell, std::vector<FF*> FFToBank);

};

#endif