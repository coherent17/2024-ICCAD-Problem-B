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
    mgr.getOverallCost(cost_verbose, 0);
    mgr.preprocess();
    mgr.getOverallCost(cost_verbose, 0);
    mgr.dumpVisual("Preprocessor.out");

    // mgr.meanshift();
    // mgr.getOverallCost(cost_verbose);
    // mgr.dumpVisual("Meanshift.out");

    mgr.preLegalize();
    mgr.getOverallCost(cost_verbose, 1);
    mgr.dumpVisual("PreLegalize.out");

    mgr.banking();
    mgr.getOverallCost(cost_verbose, 0);
    mgr.dumpVisual("Banking.out");

    mgr.postBankingOptimize();
    mgr.getOverallCost(cost_verbose, 0);
    mgr.dumpVisual("PostCG.out");

    mgr.legalize();
    mgr.getOverallCost(cost_verbose, 1);
    mgr.dumpVisual("Legalize.out");
    mgr.checker();

    mgr.detailplacement();
    mgr.getOverallCost(cost_verbose, 1);
    mgr.dumpVisual("DetailPlacement.out");
    mgr.checker();

    mgr.dump(argv[2]);
    return 0;
}
