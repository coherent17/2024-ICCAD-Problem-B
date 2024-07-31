#include "PostBankingOptimizer.h"
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
    std::vector<FF*> FFs(mgr.FF_Map.size());
    size_t i=0;
    for(auto& FF_m : mgr.FF_Map){
        FFs[i] = FF_m.second;
        i++;
    }

    vector<Coor> oldCoor(FFs.size());
    for(size_t j=0;j<FFs.size();j++)
        oldCoor[j] = FFs[j]->getNewCoor();
    postBankingObjFunction obj(mgr, mgr.FF_Map, idx_map, totalFF, FFs);
    const double kAlpha = mgr.Bit_FF_Map[1][0]->getW();
    Gradient optimizer(mgr, mgr.FF_Map, obj, kAlpha, idx_map, mgr.FF_Map.size(), FFs);

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
        if(abs(newTNS - prevTNS) / abs(prevTNS) < terminateThreshold || newTNS >= prevTNS){
            std::cout << "\n\nGradient Convergen at " << i << " iteration." << std::endl;
            std::cout << "Final statistic" << std::endl;
            std::cout << "\tTNS : " << mgr.getTNS() << std::endl;
            if(newTNS > prevTNS && i==0)
                for(size_t j=0;j<FFs.size();j++){
                    FFs[j]->setNewCoor(oldCoor[j]);
                    FFs[j]->setCoor(oldCoor[j]);
                }
            break;
        }
        prevTNS = newTNS;
    }
}

