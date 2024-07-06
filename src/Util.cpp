#include "Util.h"

double SquareEuclideanDistance(const Coor &p1, const Coor &p2){
    return std::pow(p1.getX() - p2.getX(), 2) + std::pow(p1.getY() - p2.getY(), 2);
}


double MangattanDistance(const Coor &p1, const Coor &p2){
    return std::abs(p1.getX() - p2.getX()) + std::abs(p1.getY() - p2.getY());
}

double GaussianKernel(const Coor &p1, const Coor &p2, double bandwidth){
    double numerator = SquareEuclideanDistance(p1, p2);
    double denominator = -2 * std::pow(bandwidth, 2);
    return std::exp(numerator / denominator);

}

// string getNewFFName(const string& prefix, const int& count){
//     assert("number of FF exceed INT_MAX, pls modify counter datatype" && count != INT_MAX);
//     return prefix + to_string(count);
// }

double HPWL(const Coor& c1, const Coor& c2){
    return std::abs(c1.x - c2.x) + std::abs(c1.y - c2.y);
}

/*
double HPWL(const Net& n, vector<Coor>& c, Manager& mgr){
    // get the HPWL of current Net
    // ignore : because newest QA state that HPWL is pin to pin
    double max_x = INT_MIN; // WARNING :: DBL_MAX cannot use!!! should be solve
    double max_y = INT_MIN;
    double min_x = INT_MAX;
    double min_y = INT_MAX;
    for(int j=0;j<n.getNumPins();j++){
        const Pin& p = n.getPin(j);
        string instName = p.getInstanceName();
        double x, y;
        if(mgr.FF_Map.count(instName)){
            int id = mgr.FF_Map[instName].getIdx();
            x = c[id].x;
            y = c[id].y;
        }
        else{
            x = mgr.FF_Map[instName].getCoor().x;
            y = mgr.FF_Map[instName].getCoor().y;
        }
        max_x = max(max_x, x);
        max_y = max(max_y, y);
        min_x = min(min_x, x);
        min_y = min(min_y, y);
    }
    return (max_x - min_x) + (max_y - min_y);
}
*/
