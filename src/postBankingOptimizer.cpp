#include "postBankingOptimizer.h"
#include "OptimalLocation.h"

postBankingOptimizer::postBankingOptimizer(Manager& mgr) : mgr(mgr){

}

postBankingOptimizer::~postBankingOptimizer(){

}

void postBankingOptimizer::run(){
    std::cout << "[Post banking optimize]" << std::endl;
    // create FF logic
    int totalFF = mgr.preprocessor->getFFList().size();
    std::unordered_map<std::string, int> idx_map;
    int i=0;
    for(auto&FF_m : mgr.FF_Map){ // initial location and set index
        idx_map[FF_m.second->getInstanceName()] = i;
        i++;
    }


    postBankingObjFunction obj(mgr, mgr.FF_Map, idx_map, totalFF);
    const double kAlpha = 100;
    Gradient optimizer(mgr, mgr.FF_Map, obj, kAlpha, idx_map, mgr.FF_Map.size());

    std::cout << "Slack statistic before Optimize" << std::endl;
    std::cout << "\tTNS : " << mgr.getTNS() << std::endl;
    double prevTNS = mgr.getTNS();
    const double terminateThreshold = 0.001;
    for(i=0;i<=1000;i++){
        optimizer.Step(true);
        
        // update original data
        if(i % 25 == 0){
            std::cout << "\nphase 1 step : " << i << std::endl;
            std::cout << "Slack statistic after Optimize" << std::endl;
            std::cout << "\tTNS : " << mgr.getTNS() << std::endl;
        }
        double newTNS = mgr.getTNS();
        if(abs(newTNS - prevTNS) / abs(prevTNS) < terminateThreshold || newTNS == prevTNS){
            std::cout << "\n\nGradient Convergen at " << i << " iteration." << std::endl;
            std::cout << "Final statistic" << std::endl;
            std::cout << "\tTNS : " << mgr.getTNS() << std::endl;
            break;
        }
        prevTNS = newTNS;
    }
}

