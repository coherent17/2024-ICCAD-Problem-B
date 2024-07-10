#include "Banking.h"

Banking::Banking(Manager& mgr) : mgr(mgr){}

Banking::~Banking(){}

void Banking::run(){
    std::cout << "Running cluster..." << std::endl;
    libScoring();
}

void Banking::libScoring(){
    std::map<int, std::vector<Cell *>> bit_map(mgr.Bit_FF_Map.begin(), mgr.Bit_FF_Map.end());
    for(auto &pair: bit_map){
        std::vector<Cell *> &cell_vector = pair.second;
        for(size_t i = 0; i < cell_vector.size(); i++){
            double area = cell_vector[i]->getW() * cell_vector[i]->getH();
            double score = mgr.alpha*cell_vector[i]->getQpinDelay() + mgr.beta*cell_vector[i]->getGatePower() + mgr.gamma*area;
            cell_vector[i]->setScore(score);
        }
        sortCell(cell_vector);
        //DEBUG
        // for(size_t i = 0; i < pair.second.size(); i++){
        //     std::cout << pair.second[i]->getCellName() << ": " << pair.second[i]->getScore() << std::endl;
        // }
    }
}

void Banking::sortCell(std::vector<Cell *> &cell_vector){
    auto scoreCmp = [](const Cell * cell1, const Cell * cell2){
        return cell1->getScore() > cell2->getScore();
    };
    std::sort(cell_vector.begin(), cell_vector.end(), scoreCmp);
}

Coor Banking::getMedian(FF* nowFF, Cluster& c, std::vector<PointWithID>& resultFFs, std::vector<PointWithID>& toRemoveFFs){
    std::vector<double> median_x, median_y;
    for(size_t i = 0; i < resultFFs.size(); i++){
        double dis = SquareEuclideanDistance (nowFF->getNewCoor(), mgr.FFs[resultFFs[i].second]->getNewCoor());
        if (dis < SQUARE_EPSILON && !mgr.FFs[resultFFs[i].second]->getIsCluster())
        {
            median_x.push_back (mgr.FFs[resultFFs[i].second]->getNewCoor().x);
            median_y.push_back (mgr.FFs[resultFFs[i].second]->getNewCoor().y);
            toRemoveFFs.push_back (resultFFs[i]);
            c.addFF (mgr.FFs[resultFFs[i].second]);
        }
    }
    assert (!toRemoveFFs.empty());
    std::sort (median_x.begin(), median_x.end());
    std::sort (median_y.begin(), median_y.end());
    double x = median_x[(int) toRemoveFFs.size() / 2];
    double y = median_y[(int) toRemoveFFs.size() / 2];
    return Coor(x,y);
    
}

void Banking::doClustering(){
    std::vector<PointWithID> points;
    points.reserve(mgr.FF_Map.size());
    for(size_t i = 0; i < mgr.FFs.size(); i++){
        FF *ff = mgr.FFs[i];
        points.push_back(std::make_pair(Point(ff->getCoor().x,ff->getCoor().y), i));
    }
    bgi::rtree< PointWithID, bgi::quadratic<P_PER_NODE> > rtree;
    rtree.insert(points.begin(), points.end());
    
    int clusterNum = 0;
    for (size_t index = 0; index < mgr.FFs.size(); index++){
        FF* nowFF = mgr.FFs[index];
        if (nowFF->getIsCluster()) {continue;}
        Cluster c;
        std::vector<PointWithID> resultFFs, toRemoveFFs;
        resultFFs.reserve(mgr.MaxBit);
        toRemoveFFs.reserve(mgr.MaxBit);
        rtree.query(bgi::nearest(Point(nowFF->getNewCoor().x, nowFF->getNewCoor().y), mgr.MaxBit), std::back_inserter(resultFFs));

        Coor clusterCoor = getMedian(nowFF, c, resultFFs, toRemoveFFs);
        c.setCoor(clusterCoor);
        for (size_t j = 0; j < toRemoveFFs.size(); j++)
        {
            FF* toRemoveFF = mgr.FFs[toRemoveFFs[j].second];
            toRemoveFF->setClusterIdx(clusterNum);
            toRemoveFF->setNewCoor(clusterCoor);
        }
        rtree.remove (toRemoveFFs.begin(), toRemoveFFs.end());
        mgr.clusters.push_back (c);
        clusterNum++;
    }
}
    