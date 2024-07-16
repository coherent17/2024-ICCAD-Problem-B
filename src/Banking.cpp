#include "Banking.h"

Banking::Banking(Manager& mgr) : mgr(mgr){}

Banking::~Banking(){}

void Banking::run(){
    std::cout << "Running cluster..." << std::endl;
    for(auto &pair: mgr.FF_Map){
        FF* ff = pair.second;
        double TNS, WNS;
        ff->updateSlack(mgr);
        ff->getNS(TNS, WNS);
        if(TNS > 0)
            std::cout << TNS << std::endl;
    }
    libScoring();
    doClustering();
    ClusterResult();
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
        return cell1->getScore() < cell2->getScore();
    };
    std::sort(cell_vector.begin(), cell_vector.end(), scoreCmp);
}

Cell* Banking::chooseCandidateFF(FF* nowFF, Cluster& c, std::vector<PointWithID>& resultFFs, std::vector<PointWithID>& toRemoveFFs, std::vector<FF*> &FFToBank){
    std::vector<std::pair<int, double>> nearFFs;
    PointWithID curFF;
    for(int i = 0; i < (int)resultFFs.size(); i++){
        double dis = SquareEuclideanDistance (nowFF->getNewCoor(), FFs[resultFFs[i].second]->getNewCoor());
        if(dis == 0){
            curFF = resultFFs[i];
        }
        else if (dis < SQUARE_EPSILON && nowFF->getClkIdx() == FFs[resultFFs[i].second]->getClkIdx())
        {   
            FFs[resultFFs[i].second]->updateSlack(mgr);            
            double slack = FFs[resultFFs[i].second]->getTimingSlack("D") - std::sqrt(dis)*mgr.DisplacementDelay;
            if(slack > 0){
                // std::cout << "slack:" << slack << std::endl;
                nearFFs.push_back ({i, slack});
            }
                
        }
    }
    // assert (!nearFFs.empty());
    if(nearFFs.size() > 0){
        sortFFs(nearFFs);
        Cell* chooseCell = chooseCellLib(nearFFs.size()+1);
        int bitNum = chooseCell->getBits();

        toRemoveFFs.push_back(curFF);
        FFToBank.push_back(FFs[curFF.second]);
        c.addFF (FFs[curFF.second]->getInstanceName());
        for(int i = 0; i < bitNum-1; i++){
            toRemoveFFs.push_back (resultFFs[nearFFs[i].first]);
            FFToBank.push_back(FFs[resultFFs[nearFFs[i].first].second]);
            c.addFF (FFs[resultFFs[nearFFs[i].first].second]->getInstanceName());
        }
        c.setCell(chooseCell);
        return chooseCell;
    }
    return nullptr;
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
        median_x.push_back (FFs[toRemoveFFs[i].second]->getNewCoor().x);
        median_y.push_back (FFs[toRemoveFFs[i].second]->getNewCoor().y);
    }
    std::sort (median_x.begin(), median_x.end());
    std::sort (median_y.begin(), median_y.end());
    double x = median_x[(int) toRemoveFFs.size() / 2];
    double y = median_y[(int) toRemoveFFs.size() / 2];
    return Coor(x,y);
}


void Banking::sortFFs(std::vector<std::pair<int, double>> &nearFFs){
    auto FFcmp = [](const std::pair<int, double> &neighbor1, const std::pair<int, double> &neighbor2){
        return neighbor1.second > neighbor2.second;
    };
    std::sort(nearFFs.begin(), nearFFs.end(), FFcmp);
}

void Banking::doClustering(){
    std::vector<PointWithID> points;
    points.reserve(mgr.FF_Map.size());

    // make unique id for the flipflop
    for(const auto &pair : mgr.FF_Map){
        FFs.push_back(pair.second);
    }

    for(size_t i = 0; i < FFs.size(); i++){
        FF *ff = FFs[i];
        points.push_back(std::make_pair(Point(ff->getNewCoor().x,ff->getNewCoor().y), i));
    }
    bgi::rtree< PointWithID, bgi::quadratic<P_PER_NODE> > rtree;
    rtree.insert(points.begin(), points.end());
    std::vector<bool> isClustered (FFs.size(), false);
    
    int clusterNum = 0;
    for (size_t index = 0; index < FFs.size(); index++){
        FF* nowFF = FFs[index];
        if (isClustered[index]) {continue;}
        Cluster c;  // DEBUG
        std::vector<PointWithID> resultFFs, toRemoveFFs;
        resultFFs.reserve(mgr.MaxBit);
        rtree.query(bgi::nearest(Point(nowFF->getNewCoor().x, nowFF->getNewCoor().y), mgr.MaxBit), std::back_inserter(resultFFs));
        std::vector<FF*> FFToBank;
        Cell* chooseCell = chooseCandidateFF(nowFF, c, resultFFs, toRemoveFFs, FFToBank);
        
        
        if(!toRemoveFFs.empty()){
            Coor clusterCoor = getMedian(toRemoveFFs);
            mgr.bankFF(clusterCoor, chooseCell, FFToBank);
            c.setCoor(clusterCoor);
            for (size_t j = 0; j < toRemoveFFs.size(); j++)
            {
                isClustered[toRemoveFFs[j].second] = true;
                FF* toRemoveFF = FFs[toRemoveFFs[j].second];
                toRemoveFF->setClusterIdx(clusterNum);
                toRemoveFF->setNewCoor(clusterCoor);
            }
            rtree.remove (toRemoveFFs.begin(), toRemoveFFs.end());
            clusters.push_back (c);
            clusterNum++;
        }
        
    }
    
}

void Banking::ClusterResult(){
    std::cout << "Cluster Num: " << clusters.size() << std::endl;
    // DEBUG
    // for(size_t i = 0; i < clusters.size(); i++){
    //     std::cout << clusters[i].getCell()->getCellName() << ":";
    //     std::vector<std::string> FFsName = clusters[i].getFFsName();
    //     for(size_t j = 0; j < FFsName.size(); j++){
    //         std::cout << " " << FFsName[j];
    //     }
    //     std::cout << std::endl;
    // }
    // for(const auto &MBFF : mgr.FF_Map){
    //     std::vector<FF*> clusterFFs = MBFF.second->getClusterFF();
    //     std::cout << MBFF.second->getInstanceName() << ":";
    //     if(clusterFFs.size() != 0){
    //         for(size_t i = 0; i < clusterFFs.size(); i++){
    //             std::cout << " " << clusterFFs[i]->getInstanceName();
    //         }
    //     }else{
    //         std::cout << " No Cluster"; 
    //     }
    //     std::cout << std::endl;
    // }

    // give to legalizer
    for(const auto &pair : mgr.FF_Map){
        mgr.FFs.push_back(pair.second);
    }

}
    