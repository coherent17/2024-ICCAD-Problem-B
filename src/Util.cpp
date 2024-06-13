#include "Util.h"

double EuclideanDistance(const Coor &p1, const Coor &p2){
    return pow(p1.getX() - p2.getX(), 2) + pow(p1.getY() - p2.getY(), 2);
}

double MangattanDistance(const Coor &p1, const Coor &p2){
    return abs(p1.getX() - p2.getX()) + abs(p1.getY() - p2.getY());
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