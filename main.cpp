#include "Manager.h"
#include <iostream>
#include <vector>

int main(int argc, char *argv[]){

    std::cout << std::fixed << std::setprecision(2);

    if (argc < 2){
        std::cerr << "Usage: " << argv[0] << " <inputfile> <outputfile>" << std::endl;
        return EXIT_FAILURE;
    }

    bool cost_verbose = true;

    Manager mgr;
    mgr.parse(argv[1]);
    mgr.libScoring();
    mgr.getOverallCost(cost_verbose);
    mgr.preprocess();
    std::cout << "Slack after preprocess" << std::endl;
    mgr.getOverallCost(cost_verbose);

    mgr.meanshift();
    std::cout << "Slack after meanshift" << std::endl;
    mgr.getOverallCost(cost_verbose);

    mgr.banking();
    std::cout << "Slack after banking" << std::endl;
    mgr.getOverallCost(cost_verbose);

    mgr.postBankingOptimize();
    std::cout << "Slack after postBankingOptimize" << std::endl;
    mgr.getOverallCost(cost_verbose);

    // if(argc == 4){
    //     mgr.dumpVisual(argv[3]);
    // }

    // place holder method for runing legalize algorithm
    mgr.legalize();
    mgr.getOverallCost(cost_verbose);
    if(argc == 4){
        mgr.dumpVisual(argv[3]);
    }
    mgr.dump(argv[2]);
    return 0;
}