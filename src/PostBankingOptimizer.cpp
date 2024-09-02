#include "PostBankingOptimizer.h"
#include "OptimalLocation.h"

postBankingOptimizer::postBankingOptimizer(Manager& mgr) : mgr(mgr){

}

postBankingOptimizer::~postBankingOptimizer(){

}

void postBankingOptimizer::run(){
    #ifndef NDEBUG
    std::cout << "[Post banking optimize]" << std::endl;
    #endif
    // create FF logic
    std::unordered_map<std::string, int> idx_map;
    std::vector<FF*> FFsFixed;
    std::vector<FF*> FFs(mgr.FF_Map.size());
    FFsFixed.reserve(mgr.FF_Map.size());
    size_t i=0;

    for(auto& FF_m : mgr.FF_Map){
        FF* curFF = FF_m.second;
        // if(1 || !curFF->getFixed()){ // ignore fixed idea, it does improve :( 
        if(!curFF->getIsLegalize()){
            FFsFixed.push_back(curFF);
        }
        FFs[i] = curFF;
        i++;
    }

    vector<Coor> oldCoor(FFsFixed.size());
    for(size_t j=0;j<FFsFixed.size();j++)
        oldCoor[j] = FFsFixed[j]->getNewCoor();
    postBankingObjFunction obj(mgr, mgr.FF_Map, idx_map, FFsFixed.size(), FFsFixed);
    const double kAlpha = mgr.Bit_FF_Map[1][0]->getW();
    Gradient optimizer(mgr, mgr.FF_Map, obj, kAlpha, idx_map, FFsFixed.size(), FFsFixed);

    #ifndef NDEBUG
    std::cout << "Slack statistic before Optimize" << std::endl;
    std::cout << "\tTNS : " << mgr.getTNS() << std::endl;
    #endif
    double prevTNS = mgr.getTNS();
    const double terminateThreshold = 0.001;
    for(i=0;i<=1000;i++){
        optimizer.Step(true);
        
        // update original data
        if(i % 25 == 0){
            #ifndef NDEBUG
            std::cout << "\nphase 1 step : " << i << std::endl;
            std::cout << "Slack statistic after Optimize" << std::endl;
            std::cout << "\tTNS : " << mgr.getTNS() << std::endl;
            #endif
        }
        double newTNS = mgr.getTNS();
        if(abs(newTNS - prevTNS) / abs(prevTNS) < terminateThreshold || newTNS >= prevTNS){
            #ifndef NDEBUG
            std::cout << "\n\nGradient Convergen at " << i << " iteration." << std::endl;
            std::cout << "Final statistic" << std::endl;
            std::cout << "\tTNS : " << mgr.getTNS() << std::endl;
            #endif
            if(newTNS > prevTNS && i==0)
                for(size_t j=0;j<FFsFixed.size();j++){
                    FFsFixed[j]->setNewCoor(oldCoor[j]);
                    FFsFixed[j]->setCoor(oldCoor[j]);
                }
            break;
        }
        prevTNS = newTNS;
    }
}

