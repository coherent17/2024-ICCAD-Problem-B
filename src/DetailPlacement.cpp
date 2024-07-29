#include "DetailPlacement.h"

DetailPlacement::DetailPlacement(Manager &mgr) : mgr(mgr){
    this->legalizer = mgr.legalizer;
}

DetailPlacement::~DetailPlacement(){

}

void DetailPlacement::run(){
    DEBUG_DP("Running detail placement!");
    BuildRtreeMaps();
    //GlobalSwap();
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
    // std::sort(legalizer->ffs.begin(), legalizer->ffs.end(), [](const Node *a, const Node *b){
    //     return a->getTNS() > b->getTNS();
    // });

    for(size_t i = 0; i < legalizer->ffs.size() / 2; i++){
    // for(const auto &ff : legalizer->ffs){
        Node *ff = legalizer->ffs[i];
        std::vector<std::pair<Coor, double>> Neighbors = mgr.FF_Map[ff->getName()]->getCriticalCoor();
        Coor optCoor(0, 0);
        double NegativeSlackSum = 0;
        for(size_t i = 0; i < Neighbors.size(); i++){
            Coor coor = Neighbors[i].first;
            double slack = Neighbors[i].second - 50;
            if(slack < 0){
                optCoor.x += std::abs(slack) * coor.x;
                optCoor.y += std::abs(slack) * coor.y;
                NegativeSlackSum += std::abs(slack);
            }
        }

        // ignore the positive slack
        if(NegativeSlackSum == 0) break;
        optCoor.x = optCoor.x / NegativeSlackSum;
        optCoor.y = optCoor.y / NegativeSlackSum;

        // Perform a nearest neighbor search
        std::vector<PointWithID> nearestResults;
        Point queryPoint(optCoor.x, optCoor.y);
        RtreeMaps[ff->getCell()].query(bgi::nearest(queryPoint, 5), std::back_inserter(nearestResults));
        const auto& nearestPoint = nearestResults[0];
        if(nearestPoint.first.get<0>() == ff->getLGCoor().x && nearestPoint.first.get<1>() == ff->getLGCoor().y){
            RtreeMaps[ff->getCell()].remove(nearestPoint);
            continue;
        }

        Node *ff_current = ff;
        Node *ff_choose_to_swap = legalizer->ffs[nearestPoint.second];
        Coor origin_current_coor = ff_current->getLGCoor();
        mgr.FF_Map[ff_choose_to_swap->getName()]->setNewCoor(ff->getLGCoor());
        mgr.FF_Map[ff_current->getName()]->setNewCoor(ff_choose_to_swap->getLGCoor());
        ff_current->setLGCoor(ff_choose_to_swap->getLGCoor());
        ff_choose_to_swap->setLGCoor(origin_current_coor);

        // std::cout << ff->getGPCoor() << std::endl;
        // std::cout << ff->getLGCoor() << std::endl;
        // std::cout << optCoor << std::endl;
        // std::cout << nearestPoint.first.get<0>() << ", " << nearestPoint.first.get<1>() << std::endl;
        RtreeMaps[ff->getCell()].remove(nearestPoint);
        RtreeMaps[ff->getCell()].remove(nearestPoint);
    }
}

void DetailPlacement::VerticalSwap(){

}

void DetailPlacement::LoaclReordering(){

}