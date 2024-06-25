#ifndef _MEAN_SHIFT_H_
#define _MEAN_SHIFT_H_

#define BOOST_ALLOW_DEPRECATED_HEADERS
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/foreach.hpp>
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

class MeanShift{
private:
    RTree rtree;

public:
    MeanShift();
    ~MeanShift();

    // main driven code for Mean Shift
    void run();

private:
    void buildRtree();
    void initKNN();
    void shiftFFs();
};

#endif