#include "MeanShift.h"

MeanShift::MeanShift(){}

MeanShift::~MeanShift(){}

void MeanShift::run(Manager &mgr){
    DEBUG_MSG("RUN RTREE")
    buildRtree(mgr);
    DEBUG_MSG("RUN KNN")
    initKNN(mgr);
    DEBUG_MSG("SHIFT FF")
    shiftFFs(mgr);
}

void MeanShift::buildRtree(Manager &mgr){
    std::vector<PointWithID> pointwithids;
    pointwithids.reserve(mgr.FF_Map.size());

    // make unique id for the flipflop
    for(const auto &pair : mgr.FF_Map){
        mgr.FFs.push_back(pair.second);
    }

    for(size_t i = 0; i < mgr.FFs.size(); i++){
        FF *ff = mgr.FFs[i];
        ff->setFFIdx(i);
        pointwithids.push_back(std::make_pair(Point(ff->getCoor().x,ff->getCoor().y), i));
    }
    rtree.insert(pointwithids.begin(), pointwithids.end());
}

void MeanShift::initKNN(Manager &mgr){

    #pragma omp parallel for num_threads(MAX_THREADS)
    for(size_t i = 0; i < mgr.FFs.size(); i++){
        FF *ff = mgr.FFs[i];
        ff->setNewCoor(ff->getCoor());
        FFrunKNN(mgr, i);
        
        // no need to shift
        if(ff->getNeighborSize() <= 1){
            ff->setIsShifting(false);
        }
        else{
            ff->setBandwidth();
        }
    }
}

void MeanShift::shiftFFs(Manager &mgr){
    iterationCount.resize(mgr.FFs.size());
    for (size_t i = 0; i < mgr.FFs.size(); i++)
    {
        iterationCount[i] = {0, i};
    }

    for(size_t i = 0; i < mgr.FFs.size(); i++){
        FF *ff = mgr.FFs[i];
        if(!ff->getIsShifting()){
            iterationCount[i].first = 0;
            continue;
        }
        int iteration = 0;
        while(++iteration){
            double distance = ff->shift(mgr);
            if(distance < SHIFT_TOLERANCE){
                ff->setIsShifting(false);
                iterationCount[i].first = iteration;
                break;
            }
            std::cout << iteration << std::endl;
        }
    }
}

void MeanShift::FFrunKNN(const Manager &mgr, int ffidx){
    FF *ff = mgr.FFs[ffidx];
    std::vector<PointWithID> neighbors;
    rtree.query(bgi::nearest(Point(ff->getCoor().x, ff->getCoor().y), MAX_NEIGHBORS), std::back_inserter(neighbors));
    BOOST_FOREACH(PointWithID const &p, neighbors){
        int ffneighbor_idx = p.second;
        FF *ffneighbor = mgr.FFs[ffneighbor_idx];
        double distance = SquareEuclideanDistance(ff->getCoor(), ffneighbor->getCoor());
        if(distance < MAX_SQUARE_DISPLACEMENT){
            ff->addNeighbor(ffneighbor_idx, distance);
        }
    }
    ff->sortNeighbors();
}