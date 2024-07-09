#include "Banking.h"

Banking::Banking(Manager& mgr) : mgr(mgr){}

Banking::~Banking(){}

void Banking::run(){
    std::cout << "Running cluster..." << std::endl;
}

Coor Banking::getMedian(FF* nowFF, Cluster& c, std::vector<int>& resultFFs, std::vector<int>& toRemoveFFs){
    std::vector<double> median_x, median_y;
    for(size_t i = 0; i < resultFFs.size(); i++){
        double dis = SquareEuclideanDistance (nowFF->getNewCoor(), mgr.FFs[resultFFs[i]]->getNewCoor());
        if (dis < SQUARE_EPSILON)
        {
            median_x.push_back (mgr.FFs[resultFFs[i]]->getNewCoor().x);
            median_y.push_back (mgr.FFs[resultFFs[i]]->getNewCoor().y);
            toRemoveFFs.push_back (resultFFs[i]);
            c.addFF (mgr.FFs[resultFFs[i]]);
        }
    }
    std::sort (median_x.begin(), median_x.end());
    std::sort (median_y.begin(), median_y.end());
    double x = median_x[(int) toRemoveFFs.size() / 2];
    double y = median_y[(int) toRemoveFFs.size() / 2];
    return Coor(x,y);
    
}
    