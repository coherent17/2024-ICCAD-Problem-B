#include "Banking.h"

Banking::Banking(Manager& mgr) : mgr(mgr){
    for(const auto &bitLib : mgr.Bit_FF_Map){
        clusterNum[bitLib.first] = 0;
    }
}

Banking::~Banking(){}

void Banking::run(){
    DEBUG_BAN("Running cluster...");
    bitOrdering();
    Timer t = Timer();
    t.start();
    if(bitOrder[0] != 1){
        doClustering();
    }
    t.stop();
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
    DEBUG_BAN("[LIB MBFF SCORE]");

    for(auto const& bit_pair: bitScoreVector){
        bitOrder.push_back(bit_pair.second);
        // DEBUG
        DEBUG_BAN("\t\t" + mgr.Bit_FF_Map[bit_pair.second][0]->getCellName() + "(" + std::to_string(bit_pair.second)  +  "): " + std::to_string(bit_pair.first));
    }
}

bool Banking::chooseCandidateFF(FF* nowFF, std::vector<PointWithID>& resultFFs, std::vector<PointWithID>& toRemoveFFs, std::vector<FF*> &FFToBank, const int &targetBit){
    std::vector<std::pair<int, double>> nearFFs;
    PointWithID curFF;
    for(int i = 0; i < (int)resultFFs.size(); i++){
        FF* ff = FFs[resultFFs[i].second];
        double dis = HPWL(nowFF->getNewCoor(), ff->getNewCoor());
        if(dis == 0){
            curFF = resultFFs[i];
        }
        else{          
            // ff->updateSlack();     
            // double slack = FFs[resultFFs[i].second]->getTimingSlack("D") - std::sqrt(dis)*mgr.DisplacementDelay;
            nearFFs.push_back ({i, dis});
        }
    }

    if(nearFFs.size() > 0){
        sortFFs(nearFFs);
        toRemoveFFs.push_back(curFF);
        FFToBank.push_back(FFs[curFF.second]);
        int bitSum = FFs[curFF.second]->getCell()->getBits();
        int i = 0;
        while(bitSum < targetBit && i < (int)nearFFs.size()){
            PointWithID ffPoint = resultFFs[nearFFs[i].first];
            FF* ff = FFs[ffPoint.second];
            if(bitSum + ff->getCell()->getBits() <= targetBit){
                toRemoveFFs.push_back(ffPoint);
                FFToBank.push_back(ff);
                bitSum += ff->getCell()->getBits();
            }
            i++;
        }
        if(bitSum == targetBit) 
            return true;
        else 
            return false;
    }
    return false;
}

// // Can construct the LUT first...
// Cell* Banking::chooseCellLib(int bitNum){
//     int order = 0;
//     int targetBit = bitOrder[order];
//     while(bitNum != targetBit && order < (int)bitOrder.size()){
//         if(targetBit > bitNum){
//             order++;
//             targetBit = bitOrder[order];
//             continue;
//         }
//         bitNum--;
//     }
//     assert(bitNum > 0);
//     return mgr.Bit_FF_Map[bitNum][0];
// }

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
        return neighbor1.second < neighbor2.second;
    };
    std::sort(nearFFs.begin(), nearFFs.end(), FFcmp);
}

double Banking::CostCompare(const Coor clusterCoor, Cell* chooseCell, std::vector<FF*> FFToBank){
    double costOptimize = 0;
    for(size_t i = 0; i < FFToBank.size(); i++){
        FF* ff = FFToBank[i];
        //costOptimize += mgr.alpha * (ff->getCell()->getQpinDelay());
        costOptimize += mgr.beta * (ff->getCell()->getGatePower());
        costOptimize += mgr.gamma * (ff->getCell()->getArea());
    }
    costOptimize -= mgr.beta * (chooseCell->getGatePower()) + mgr.gamma * (chooseCell->getArea());
    double increaseTNS = 0;
    for(size_t i = 0; i < FFToBank.size(); i++){
        FF* ff = FFToBank[i];
        int affectNum = 1;
        for(const auto & clusterFF : ff->getClusterFF()){
            affectNum += clusterFF->getNextStage().size();
            costOptimize += (ff->getCell()->getQpinDelay() - chooseCell->getQpinDelay()) * clusterFF->getNextStage().size();
        }
        double displacementAffect = mgr.DisplacementDelay * HPWL(ff->getNewCoor(), clusterCoor) * affectNum;
        increaseTNS += displacementAffect;
    }
    costOptimize -= mgr.alpha * increaseTNS;
    // if(costOptimize > -100 && costOptimize < 0) std::cout << costOptimize << std::endl;
    return costOptimize;
}

void Banking::doClustering(){
    int clusterTotalNum = 0;
    // make unique id for the flipflop
    size_t max_clk_idx = 0;
    for(const auto &pair : mgr.FF_Map){
        max_clk_idx = std::max((int)max_clk_idx, pair.second->getClkIdx());
    }
    std::map<int, std::vector<Cell *>> orderBitMap(mgr.Bit_FF_Map.begin(), mgr.Bit_FF_Map.end());

    for(const auto &bitLib : orderBitMap){
        Cell* chooseCell = bitLib.second[0];
        int targetBit =  chooseCell->getBits();
        if(targetBit == 1) continue;
        DEBUG_BAN("Cluster " + std::to_string(targetBit) + " Bit MBFF");
        //every bit library use one legalizer
        mgr.legalizer = new Legalizer(mgr);
        mgr.legalizer->initial();

        for(size_t clkIDX = 0; clkIDX <= max_clk_idx; clkIDX++){
            FFs.clear();
            for(const auto &pair : mgr.FF_Map){
                if((size_t)pair.second->getClkIdx() == clkIDX){
                    pair.second->setIsLegalize(false);
                    FFs.push_back(pair.second);
                }
            }
            std::vector<PointWithID> points;
            points.reserve(FFs.size());

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
                std::vector<PointWithID> resultFFs, toRemoveFFs;
                resultFFs.reserve(mgr.MaxBit);
                rtree.query(bgi::nearest(Point(nowFF->getNewCoor().x, nowFF->getNewCoor().y), mgr.MaxBit), std::back_inserter(resultFFs));
                std::vector<FF*> FFToBank;
                bool isChoose = chooseCandidateFF(nowFF, resultFFs, toRemoveFFs, FFToBank, targetBit);
                

                if(isChoose){
                    
                    Coor medianCoor = getMedian(toRemoveFFs);
                    Coor clusterCoor = mgr.legalizer->FindPlace(medianCoor, chooseCell);
                    if(clusterCoor.x == DBL_MAX && clusterCoor.y == DBL_MAX) 
                        continue;

                    // if(mgr.getCostDiff(clusterCoor, chooseCell, FFToBank) > 0)
                    //     continue;
                    if(CostCompare(clusterCoor, chooseCell, FFToBank) < 0)
                        continue;

                    FF* newFF = mgr.bankFF(clusterCoor, chooseCell, FFToBank);
                    mgr.legalizer->UpdateRows(newFF);
                    newFF->setIsLegalize(true);
                    for (size_t j = 0; j < toRemoveFFs.size(); j++)
                    {
                        isClustered[toRemoveFFs[j].second] = true;
                        FF* toRemoveFF = FFs[toRemoveFFs[j].second];
                        toRemoveFF->setClusterIdx(clusterTotalNum);
                        toRemoveFF->setNewCoor(clusterCoor);
                    }
                    rtree.remove (toRemoveFFs.begin(), toRemoveFFs.end());
                    clusterTotalNum++;
                }
            }
        }     
    }
}

void Banking::restoreUnclusterFFCoor(){
    for(const auto &MBFF : mgr.FF_Map){
        std::vector<FF*> clusterFFs = MBFF.second->getClusterFF();
        if(clusterFFs.size() == 1){
            Coor originalCoor = MBFF.second->getCoor();
            MBFF.second->setNewCoor(originalCoor);
        }
    }
}

void Banking::ClusterResult(){
    for(const auto &MBFF : mgr.FF_Map){
        std::vector<FF*> clusterFFs = MBFF.second->getClusterFF();
        clusterNum[clusterFFs.size()]++;
    }
    std::map<int, int> clusterMap(clusterNum.begin(), clusterNum.end()); 
    DEBUG_BAN("[CLUSTER RESULT]");
    for(const auto &cluster : clusterMap){
        DEBUG_BAN("\t\tFF" +  std::to_string(cluster.first) + " : " + std::to_string(cluster.second));
    }
}