#include "DetailPlacement.h"

DetailPlacement::DetailPlacement(Manager &mgr) : mgr(mgr){
    this->legalizer = mgr.legalizer;
    for(const auto &ff : legalizer->ffs){
        cellSet.insert(ff->getCell());
    }
    CheckSwapSanity();
}

DetailPlacement::~DetailPlacement(){

}

void DetailPlacement::run(){
    DEBUG_DP("Running detail placement!");
    GlobalSwap();
    LocalSwap();
    GlobalSwap();
    LocalSwap();
    GlobalSwap();
    LocalSwap();
    GlobalSwap();
    LocalSwap();

    // by c119cheng
    ChangeCell();
}

void DetailPlacement::BuildGlobalRtreeMaps(){
    DEBUG_DP("Build Global Rtree");
    // init/reset rtree
    for(const auto &cell : cellSet){
        RtreeMaps[cell] = RTree();
    }

    // sort the ff by TNS
    std::sort(legalizer->ffs.begin(), legalizer->ffs.end(), [](const Node *a, const Node *b){
        return a->getTNS() > b->getTNS();
    });

    // insert ff into rtree
    for(size_t i = 0; i < legalizer->ffs.size(); i++){
        Node *ff = legalizer->ffs[i];
        PointWithID pointwithid;
        pointwithid = std::make_pair(Point(ff->getLGCoor().x, ff->getLGCoor().y), i);
        RtreeMaps[ff->getCell()].insert(pointwithid);
    }
}

void DetailPlacement::BuildLocalRtreeMaps(Node *ff){
    DEBUG_DP("Build Local Rtree");
    int ff_current_rowIdx = ff->getPlaceRowIdx();

    // init/reset rtree
    for(const auto &cell : cellSet){
        RtreeMaps[cell] = RTree();
    }

    // sort the ff by TNS
    std::sort(legalizer->ffs.begin(), legalizer->ffs.end(), [](const Node *a, const Node *b){
        return a->getTNS() > b->getTNS();
    });

    // insert ff into rtree
    std::mutex rtreeMutex;
    #pragma omp parallel for
    for(size_t i = 0; i < legalizer->ffs.size(); i++){
        Node *ffi = legalizer->ffs[i];
        // Only put the upper/lower ff into rtree
        if((int)ffi->getPlaceRowIdx() == ff_current_rowIdx + 1 || (int)ffi->getPlaceRowIdx() == ff_current_rowIdx - 1 || (int)ffi->getPlaceRowIdx() == ff_current_rowIdx){
            PointWithID pointwithid;
            pointwithid = std::make_pair(Point(ffi->getLGCoor().x, ffi->getLGCoor().y), i);
            std::lock_guard<std::mutex> guard(rtreeMutex);
            RtreeMaps[ffi->getCell()].insert(pointwithid);
        }
    }
}

void DetailPlacement::CheckSwapSanity(){
    DEBUG_DP("Swap Sanity Checker");
    for(const auto &ff : legalizer->ffs){
        assert(ff->getLGCoor().y == legalizer->rows[ff->getPlaceRowIdx()]->getStartCoor().y);
        assert(cellSet.find(ff->getCell()) != cellSet.end());
    }
}

void DetailPlacement::GlobalSwap(){
    BuildGlobalRtreeMaps();
    DEBUG_DP("Global Swap");
    for(size_t id = 0; id < legalizer->ffs.size(); id++){
        Node *ff = legalizer->ffs[id];

        // Query from rtree to find the best ff that near ff's global placement coordinate
        Point queryPoint(ff->getGPCoor().x, ff->getGPCoor().y);
        std::vector<PointWithID> nearestResults;
        RtreeMaps[ff->getCell()].query(bgi::nearest(queryPoint, 1), std::back_inserter(nearestResults));
        const auto& nearestPoint = nearestResults[0];

        // Found itself
        if(nearestPoint.second == (int)id){
            RtreeMaps[ff->getCell()].remove(nearestPoint);
            continue;
        }

        size_t ff_critical = 0;
        size_t target_critical = 0;
        for(auto& curFF : ff->getFFPtr()->getClusterFF()){
            ff_critical += 1 + curFF->getNextStage().size();
        }
        for(auto& curFF : legalizer->ffs[nearestPoint.second]->getFFPtr()->getClusterFF()){
            target_critical += 1 + curFF->getNextStage().size();
        }

        // The nearest point to swap will not improve the TNS
        if(ff->getDisplacement() * ff_critical < legalizer->ffs[nearestPoint.second]->getDisplacement(ff->getLGCoor()) * target_critical){
            continue;
        }

        // [TODO]: Maintain the Row::FFOnThisRow vector for local swap to build the rtree faster
        // Swap the ff pairs by LGCoor and placeIdx
        Node *ff_current = ff;
        Node *ff_choose_to_swap = legalizer->ffs[nearestPoint.second];

        // Commit to manager
        ff_choose_to_swap->getFFPtr()->setNewCoor(ff->getLGCoor());
        ff_current->getFFPtr()->setNewCoor(ff_choose_to_swap->getLGCoor());

        // Update the LGCoor in Node
        ff_current->setLGCoor(ff_current->getFFPtr()->getNewCoor());
        ff_choose_to_swap->setLGCoor(ff_choose_to_swap->getFFPtr()->getNewCoor());

        // Update the Node::placeIdx
        size_t ff_current_placeIdx = ff->getPlaceRowIdx();
        size_t ff_choose_to_swap_placeIdx = ff_choose_to_swap->getPlaceRowIdx();
        ff->setPlaceRowIdx(ff_choose_to_swap_placeIdx);
        ff_choose_to_swap->setPlaceRowIdx(ff_current_placeIdx);

        // Remove ff current from the rtree
        RtreeMaps[ff->getCell()].remove(nearestPoint);
    }
    CheckSwapSanity();
}

void DetailPlacement::LocalSwap(){
    DEBUG_DP("Local Swap");
    for(size_t id = 0; (int)id < std::min((int)legalizer->ffs.size(), 10); id++){
        Node *ff = legalizer->ffs[id];
        if(ff->getTNS() == 0){
            continue;
        }
        BuildLocalRtreeMaps(ff);
        // Query from rtree to find the best ff that near ff's global placement coordinate
        Point queryPoint(ff->getGPCoor().x, ff->getGPCoor().y);
        std::vector<PointWithID> nearestResults;
        RtreeMaps[ff->getCell()].query(bgi::nearest(queryPoint, 1), std::back_inserter(nearestResults));
        const auto& nearestPoint = nearestResults[0];

        // Found itself
        if(nearestPoint.second == (int)id){
            RtreeMaps[ff->getCell()].remove(nearestPoint);
            continue;
        }

        size_t ff_critical = 0;
        size_t target_critical = 0;
        for(auto& curFF : ff->getFFPtr()->getClusterFF()){
            ff_critical += 1 + curFF->getNextStage().size();
        }
        for(auto& curFF : legalizer->ffs[nearestPoint.second]->getFFPtr()->getClusterFF()){
            target_critical += 1 + curFF->getNextStage().size();
        }

        // The nearest point to swap will not improve the TNS
        if(ff->getDisplacement() * ff_critical < legalizer->ffs[nearestPoint.second]->getDisplacement(ff->getLGCoor()) * target_critical){
            continue;
        }

        // [TODO]: Maintain the Row::FFOnThisRow vector for local swap to build the rtree faster
        // Swap the ff pairs by LGCoor and placeIdx
        Node *ff_current = ff;
        Node *ff_choose_to_swap = legalizer->ffs[nearestPoint.second];

        // Commit to manager
        ff_choose_to_swap->getFFPtr()->setNewCoor(ff->getLGCoor());
        ff_current->getFFPtr()->setNewCoor(ff_choose_to_swap->getLGCoor());

        // Update the LGCoor in Node
        ff_current->setLGCoor(ff_current->getFFPtr()->getNewCoor());
        ff_choose_to_swap->setLGCoor(ff_choose_to_swap->getFFPtr()->getNewCoor());

        // Update the Node::placeIdx
        size_t ff_current_placeIdx = ff->getPlaceRowIdx();
        size_t ff_choose_to_swap_placeIdx = ff_choose_to_swap->getPlaceRowIdx();
        ff->setPlaceRowIdx(ff_choose_to_swap_placeIdx);
        ff_choose_to_swap->setPlaceRowIdx(ff_current_placeIdx);

        // Remove ff current from the rtree
        RtreeMaps[ff->getCell()].remove(nearestPoint);

    }
    CheckSwapSanity();
}

void DetailPlacement::ChangeCell(){
    vector<FF*> FFs(mgr.FF_Map.size());
    size_t idx=0;
    for(auto& ff_m : mgr.FF_Map){
        FFs[idx] = ff_m.second;
        idx++;
    }
    // #pragma omp parallel for num_threads(MAX_THREADS)
    for(size_t i=0;i<FFs.size();i++){
        FF* curFF = FFs[i];
        double bestCost = curFF->getCost();
        Cell* bestCell = curFF->getCell();
        Cell* originalCell = curFF->getCell();
        size_t bit = curFF->getCell()->getBits();
        size_t bitMapSize = mgr.Bit_FF_Map[bit].size();
        // iterate through all cell type
        for(size_t j=0;j<bitMapSize;j++){
            Cell* targetCell = mgr.Bit_FF_Map[bit][j];
            Cell* curCell = curFF->getCell();
            curFF->setCell(targetCell);
            double totalCost = curFF->getCost();
            curFF->setCell(originalCell);
            // hard constraint for using smaller cell, for easier legalize, need reconsider
            if(totalCost < bestCost && (targetCell->getW() <= originalCell->getW() && targetCell->getH() <= originalCell->getH())){
                bestCost = totalCost;
                bestCell = targetCell;
            }
        }

        curFF->setCell(bestCell);
    }
}