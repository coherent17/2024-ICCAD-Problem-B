#include "Banking.h"

Banking::Banking(Manager& mgr) : mgr(mgr){}

Banking::~Banking(){}

void Banking::run(){
    std::cout << "Running cluster..." << std::endl;
    libScoring();
    doClustering();
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

void Banking::chooseCandidateFF(FF* nowFF, Cluster& c, std::vector<PointWithID>& resultFFs, std::vector<PointWithID>& toRemoveFFs){
    std::vector<std::pair<int, double>> nearFFs;
    for(int i = 0; i < (int)resultFFs.size(); i++){
        double dis = SquareEuclideanDistance (nowFF->getNewCoor(), mgr.FFs[resultFFs[i].second]->getNewCoor());
        if (dis < SQUARE_EPSILON)
        {
            nearFFs.push_back ({i, dis});
        }
    }
    assert (!nearFFs.empty());
    if(nearFFs.size() > 1){
        sortFFs(nearFFs);
        Cell* chooseCell = chooseCellLib(nearFFs.size());
        int bitNum = chooseCell->getBits();

        for(int i = 0; i < bitNum; i++){
            toRemoveFFs.push_back (resultFFs[nearFFs[i].first]);
            c.addFF (mgr.FFs[resultFFs[nearFFs[i].first].second]);
        }
        c.setCell(chooseCell);
    }
    
}

Cell* Banking::chooseCellLib(int bitNum){
    int bitMin = bitNum;
    while(mgr.Bit_FF_Map.find(bitMin) == mgr.Bit_FF_Map.end() && bitMin > 0){
        bitMin--;
    }
    assert(bitMin > 0);
    return mgr.Bit_FF_Map[bitMin][0];
}


Coor Banking::getMedian(std::vector<PointWithID>& toRemoveFFs){
    std::vector<double> median_x, median_y;
    for(size_t i = 0; i < toRemoveFFs.size(); i++){
        median_x.push_back (mgr.FFs[toRemoveFFs[i].second]->getNewCoor().x);
        median_y.push_back (mgr.FFs[toRemoveFFs[i].second]->getNewCoor().y);
    }
    std::sort (median_x.begin(), median_x.end());
    std::sort (median_y.begin(), median_y.end());
    double x = median_x[(int) toRemoveFFs.size() / 2];
    double y = median_y[(int) toRemoveFFs.size() / 2];
    return Coor(x,y);
}


void Banking::sortFFs(std::vector<std::pair<int, double>> &nearFFs){
    auto FFcmp = [](const std::pair<int, double> &neighbor1, const std::pair<int, double> &neighbor2){
        return neighbor1.second < neighbor2.second;
    };
    std::sort(nearFFs.begin(), nearFFs.end(), FFcmp);
}

void Banking::doClustering(){
    std::vector<PointWithID> points;
    points.reserve(mgr.FF_Map.size());
    for(size_t i = 0; i < mgr.FFs.size(); i++){
        FF *ff = mgr.FFs[i];
        points.push_back(std::make_pair(Point(ff->getNewCoor().x,ff->getNewCoor().y), i));
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
        rtree.query(bgi::nearest(Point(nowFF->getNewCoor().x, nowFF->getNewCoor().y), mgr.MaxBit), std::back_inserter(resultFFs));
        chooseCandidateFF(nowFF, c, resultFFs, toRemoveFFs);
        
        
        if(!toRemoveFFs.empty()){
            Coor clusterCoor = getMedian(toRemoveFFs);
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
    std::cout << "Cluster Num: " << clusterNum << std::endl;
    // for(size_t i = 0; i < mgr.clusters.size(); i++){
    //     std::cout << mgr.clusters[i].getCell()->getCellName() << ":";
    //     std::vector<FF*> FFs = mgr.clusters[i].getFFs();
    //     for(size_t j = 0; j < FFs.size(); j++){
    //         std::cout << " " << FFs[j]->getInstanceName();
    //     }
    //     std::cout << std::endl;
    // }
}
    