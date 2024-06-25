#define BOOST_ALLOW_DEPRECATED_HEADERS
#include "Manager.h"
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <iostream>
#include <vector>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

int main(int argc, char *argv[]){
    Manager mgr;
    mgr.parse(argv[1]);
    mgr.meanshift();
    //mgr.print();
    mgr.dump(argv[2]);
    // mgr.Technology_Mapping();
    // mgr.Build_Logic_FF();
    // mgr.Build_Circuit_Gragh();
    
    // mgr.preprocess();
    // mgr.optimal_FF_location();

    // ################################### test for boost ########################################
    // Define point type and R-tree type
    typedef bg::model::point<double, 2, bg::cs::cartesian> point_t;
    typedef std::pair<point_t, int> value_t; // Each value in the R-tree will be a point and an ID
    typedef bgi::rtree<value_t, bgi::quadratic<16>> rtree_t;

    // Create an R-tree
    rtree_t rtree;

    // Insert some points into the R-tree
    rtree.insert(std::make_pair(point_t(0.0, 0.0), 1));
    rtree.insert(std::make_pair(point_t(1.0, 1.0), 2));
    rtree.insert(std::make_pair(point_t(2.0, 2.0), 3));
    rtree.insert(std::make_pair(point_t(3.0, 3.0), 4));
    rtree.insert(std::make_pair(point_t(4.0, 4.0), 5));

    // Define a query point and a search radius
    point_t query_point(2.5, 2.5);
    double search_radius = 2.0;

    // Perform a spatial query to find points within the search radius
    std::vector<value_t> result_s;
    rtree.query(bgi::satisfies([&](const value_t& v) {
        return bg::distance(v.first, query_point) <= search_radius;
    }), std::back_inserter(result_s));

    // Print the results
    std::cout << "Points within " << search_radius << " units of (" 
              << bg::get<0>(query_point) << ", " << bg::get<1>(query_point) << "):" << std::endl;
    for (const auto& value : result_s) {
        std::cout << "Point (" << bg::get<0>(value.first) << ", " << bg::get<1>(value.first) 
                  << "), ID: " << value.second << std::endl;
    }
    return 0;
}