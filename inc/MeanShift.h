#ifndef _MEAN_SHIFT_H_
#define _MEAN_SHIFT_H_

#ifdef ENABLE_DEBUG_MS
#define DEBUG_MS(message) std::cout << "[MEANSHIFT] " << message << std::endl
#else
#define DEBUG_MS(message)
#endif

#define BOOST_ALLOW_DEPRECATED_HEADERS

#ifdef ENABLE_DEBUG_CHECKER
#define DEBUG_MS(message) std::cout << "[MEANSHIFT: " << message << "]" << std::endl;
#else
#define DEBUG_MS(message)
#endif

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/foreach.hpp>
#include <vector>
#include <omp.h>
#include "Util.h"
#include "FF.h"
#include "Manager.h"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
namespace bgm = boost::geometry::model;

// 2-D point with coordinate type of double in cartesian
typedef bg::model::point<double, 2, bg::cs::cartesian> Point;

// Define a Point with an ID:
typedef std::pair<Point, int> PointWithID;

// The R-tree stores PointWithID elements and uses a quadratic split algorithm for node splitting
typedef bgi::rtree<PointWithID, bgi::quadratic<P_PER_NODE>> RTree;

class FF;
class MeanShift{
private:
    RTree rtree;
    std::vector<FF *> FFs;
    std::vector<std::pair<int, int>> iterationCount;    // {iteration to coverage, ffidx}

public:
    MeanShift();
    ~MeanShift();

    // main driven code for Mean Shift
    void run(Manager &mgr);

private:
    void buildRtree(Manager &mgr);
    void initKNN(const Manager &mgr);
    void shiftFFs();
    void FFrunKNN(const Manager &mgr, int ffidx);
};

#endif