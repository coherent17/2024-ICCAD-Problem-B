#ifndef _DETAIL_PLACEMENT_H_
#define _DETAIL_PLACEMENT_H_

#define BOOST_ALLOW_DEPRECATED_HEADERS
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/foreach.hpp>
#include <mutex>
#include "Legalizer.h"
#include "Manager.h"
#include "Random.h"
#include "Hungarian.h"
#include <iostream>
#include <vector>
#include <set>

#ifdef ENABLE_DEBUG_DP
#define DEBUG_DP(message) std::cout << "[DetailPlacement] " << message << std::endl
#else
#define DEBUG_DP(message)
#endif

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
namespace bgm = boost::geometry::model;

// 2-D point with coordinate type of double in cartesian
typedef bg::model::point<double, 2, bg::cs::cartesian> Point;

// Define a Point with an ID:
typedef std::pair<Point, int> PointWithID;

// The R-tree stores PointWithID elements and uses a quadratic split algorithm for node splitting
typedef bgi::rtree<PointWithID, bgi::quadratic<P_PER_NODE>> RTree;

class Node;
class Legalizer;
class DetailPlacement{
private:
    Manager &mgr;
    Legalizer *legalizer;
    std::unordered_map<Cell *, RTree> RtreeMaps;
    std::set<Cell *> cellSet;
    Random randomNumberGenerator;

public:
    explicit DetailPlacement(Manager &mgr);
    ~DetailPlacement();
    void run();

private:
    /**
    @brief Build rtree for same cell type for all ff
    */
    void BuildGlobalRtreeMaps();

    /**
    @brief Build rtree for same cell type for Node *ff's upper/lower row
    */
    void BuildLocalRtreeMaps(Node *ff);

    /**
    @brief 
        1. Make sure the Node::LGCoor and Node::placeIdx are consistent
        2. Make sure the Node::cell is in cellSet
    */
    void CheckSwapSanity();

    // 3 main methods for detail placement
    void GlobalSwap();
    void LocalSwap();

    // 
    void DetailAssignmentMBFF(); // for same clk
    void ChangeCell();
};


#endif