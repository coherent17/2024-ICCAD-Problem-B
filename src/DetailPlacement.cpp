#include "DetailPlacement.h"

DetailPlacement::DetailPlacement(Manager &mgr) : mgr(mgr){
    this->legalizer = mgr.legalizer;
}

DetailPlacement::~DetailPlacement(){

}

void DetailPlacement::run(){
    DEBUG_DP("Running detail placement!");
    BuildRtreeMaps();
    for(size_t i = 0; i < legalizer->ffs.size(); i++){
        GlobalSwap(legalizer->ffs[i], i);
    }
}

void DetailPlacement::BuildRtreeMaps(){
    DEBUG_DP("Build Rtree for same cell type");
    // check for the unique cells
    for(const auto &ff : legalizer->ffs){
        assert(ff->getLGCoor().y == legalizer->rows[ff->getPlaceRowIdx()]->getStartCoor().y);
        cellSet.insert(ff->getCell());
        legalizer->rows[ff->getPlaceRowIdx()]->addFFs(ff);
    }

    // init rtree
    for(const auto &cell : cellSet){
        RtreeMaps[cell] = RTree();
    }

    // insert ff into rtree
    std::sort(legalizer->ffs.begin(), legalizer->ffs.end(), [](const Node *a, const Node *b){
        return a->getTNS() > b->getTNS();
    });

    for(size_t i = 0; i < legalizer->ffs.size(); i++){
        Node *ff = legalizer->ffs[i];
        PointWithID pointwithid;
        pointwithid = std::make_pair(Point(ff->getLGCoor().x, ff->getLGCoor().y), i);
        RtreeMaps[ff->getCell()].insert(pointwithid);
    }
}

void DetailPlacement::GlobalSwap(Node *ff, int id){
    Point queryPoint(ff->getGPCoor().x, ff->getGPCoor().y);
    std::vector<PointWithID> nearestResults;
    RtreeMaps[ff->getCell()].query(bgi::nearest(queryPoint, 1), std::back_inserter(nearestResults));
    const auto& nearestPoint = nearestResults[0];

    // Found itself
    if(nearestPoint.second == id){
        RtreeMaps[ff->getCell()].remove(nearestPoint);
        return;
    }

    if(ff->getDisplacement() < legalizer->ffs[nearestPoint.second]->getDisplacement(ff->getLGCoor())){
        return;
    }

    // [TODO] also need to update the placeIdx for each ff;
    Node *ff_current = ff;
    Node *ff_choose_to_swap = legalizer->ffs[nearestPoint.second];
    ff_choose_to_swap->getFFPtr()->setNewCoor(ff->getLGCoor());
    ff_current->getFFPtr()->setNewCoor(ff_choose_to_swap->getLGCoor());
    ff_current->setLGCoor(ff_current->getFFPtr()->getNewCoor());
    ff_choose_to_swap->setLGCoor(ff_choose_to_swap->getFFPtr()->getNewCoor());
    RtreeMaps[ff->getCell()].remove(nearestPoint);
}

void DetailPlacement::VerticalSwap(){

}

void DetailPlacement::LoaclReordering(){

}