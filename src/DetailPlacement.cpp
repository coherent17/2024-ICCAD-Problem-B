#include "DetailPlacement.h"

DetailPlacement::DetailPlacement(Manager &mgr) : mgr(mgr){
    this->legalizer = mgr.legalizer;
}

DetailPlacement::~DetailPlacement(){

}

void DetailPlacement::run(){
    DEBUG_DP("Running detail placement!");
    BuildRtreeMaps();
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
    for(size_t i = 0; i < legalizer->ffs.size(); i++){
        Node *ff = legalizer->ffs[i];
        PointWithID pointwithid;
        pointwithid = std::make_pair(Point(ff->getLGCoor().x, ff->getLGCoor().y), i);
        RtreeMaps[ff->getCell()].insert(pointwithid);
    }
}

void DetailPlacement::GlobalSwap(){

}

void DetailPlacement::VerticalSwap(){

}

void DetailPlacement::LoaclReordering(){
    
}