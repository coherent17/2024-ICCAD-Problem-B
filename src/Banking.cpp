#include "Banking.h"

Banking::Banking(Manager& mgr) : mgr(mgr){
    for(const auto &bitLib : mgr.Bit_FF_Map){
        clusterNum[bitLib.first] = 0;
    }
}

Banking::~Banking(){}

void Banking::run(){
    std::cout << "Running cluster..." << std::endl;
    bitOrdering();
    if(bitOrder[0] != 1){
        doClustering();
    }
    restoreUnclusterFFCoor();
    ClusterResult();
}

void Banking::bitOrdering(){
    std::vector<std::pair<double, int>> bitScoreVector;
    for(auto &pair: mgr.Bit_FF_Map){
        std::vector<Cell *> &cell_vector = pair.second;
        bitScoreVector.push_back({cell_vector[0]->getScore()/pair.first, pair.first});
    }
    
    std::sort(bitScoreVector.begin(), bitScoreVector.end());
    std::cout << "[LIB MBFF SCORE]" << std::endl;

    for(auto const& bit_pair: bitScoreVector){
        bitOrder.push_back(bit_pair.second);
        // DEBUG
        std::cout <<"         "<< bit_pair.second << ": " << bit_pair.first << std::endl;
    }
}

Cell* Banking::chooseCandidateFF(FF* nowFF, Cluster& c, std::vector<PointWithID>& resultFFs, std::vector<PointWithID>& toRemoveFFs, std::vector<FF*> &FFToBank){
    std::vector<std::pair<int, double>> nearFFs;
    PointWithID curFF;
    for(int i = 0; i < (int)resultFFs.size(); i++){
        double dis = SquareEuclideanDistance (nowFF->getNewCoor(), FFs[resultFFs[i].second]->getNewCoor());
        if(dis == 0){
            curFF = resultFFs[i];
        }
        else if (nowFF->getClkIdx() == FFs[resultFFs[i].second]->getClkIdx())
        {   
            FFs[resultFFs[i].second]->updateSlack();            
            double slack = FFs[resultFFs[i].second]->getTimingSlack("D") - std::sqrt(dis)*mgr.DisplacementDelay;
            nearFFs.push_back ({i, slack});
        }
    }

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

// Can construct the LUT first...
Cell* Banking::chooseCellLib(int bitNum){
    int order = 0;
    int targetBit = bitOrder[order];
    while(bitNum != targetBit && order < (int)bitOrder.size()){
        if(targetBit > bitNum){
            order++;
            targetBit = bitOrder[order];
            continue;
        }
        bitNum--;
    }
    assert(bitNum > 0);
    return mgr.Bit_FF_Map[bitNum][0];
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
    int clusterTotalNum = 0;
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
            if(mgr.getCostDiff(clusterCoor, chooseCell, FFToBank) > 0)
                continue;
            mgr.bankFF(clusterCoor, chooseCell, FFToBank);
            c.setCoor(clusterCoor);
            for (size_t j = 0; j < toRemoveFFs.size(); j++)
            {
                isClustered[toRemoveFFs[j].second] = true;
                FF* toRemoveFF = FFs[toRemoveFFs[j].second];
                toRemoveFF->setClusterIdx(clusterTotalNum);
                toRemoveFF->setNewCoor(clusterCoor);
            }
            rtree.remove (toRemoveFFs.begin(), toRemoveFFs.end());
            clusters.push_back (c);
            clusterTotalNum++;
        }
        
    }
    
}

void Banking::restoreUnclusterFFCoor(){
    for(const auto &MBFF : mgr.FF_Map){
        std::vector<FF*> clusterFFs = MBFF.second->getClusterFF();
        if(clusterFFs.size() == 1){
            Coor originalCoor = MBFF.second->getCoor();
            MBFF.second->setNewCoor(originalCoor);
            MBFF.second->setCell(mgr.Bit_FF_Map[1][0]);// preprocess choose better 1-bit FF?
            MBFF.second->setCellName(mgr.Bit_FF_Map[1][0]->getCellName());
        }
    }
}

void Banking::ClusterResult(){
    for(const auto &MBFF : mgr.FF_Map){
        std::vector<FF*> clusterFFs = MBFF.second->getClusterFF();
        clusterNum[clusterFFs.size()]++;
    }
    std::map<int, int> clusterMap(clusterNum.begin(), clusterNum.end()); 
    std::cout << "[CLUSTER RESULT]" << std::endl;
    for(const auto &cluster : clusterMap){
        std::cout << "        FF" << cluster.first << " : " << cluster.second << std::endl;
    }
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
}
    