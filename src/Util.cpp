#include "Util.h"

double EuclideanDistance(const Coor &p1, const Coor &p2){
    return pow(p1.getX() - p2.getX(), 2) + pow(p1.getY() - p2.getY(), 2);
}

double MangattanDistance(const Coor &p1, const Coor &p2){
    return abs(p1.getX() - p2.getX()) + abs(p1.getY() - p2.getY());
}

double HPWL(const Net& n, vector<Coor>& c, Manager& mgr){
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

void optimal_FF_location(Manager& mgr){
    // create FF logic
    vector<Coor> c(mgr.FF_Map.size());
    int i=0;
    for(auto&FF_m : mgr.FF_Map){ // initial location and set index
        FF_m.second.setIdx(i);
        c[i].x = FF_m.second.getCoor().x;
        c[i].y = FF_m.second.getCoor().y;
        i++;
    }

    obj_function obj(mgr);
    const double kAlpha = 0.01;
    Gradient optimizer(obj, c, kAlpha);

    for(i=0;i<25;i++){
        optimizer.Step();
        // CAL new slack

    }
}
