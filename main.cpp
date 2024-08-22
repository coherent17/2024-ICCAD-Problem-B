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
    mgr.getOverallCost(cost_verbose);
    mgr.dumpVisual("Preprocessor.out");

    // mgr.meanshift();
    // mgr.getOverallCost(cost_verbose);
    // mgr.dumpVisual("Meanshift.out");

    mgr.preLegalize();
    mgr.getOverallCost(cost_verbose);
    mgr.dumpVisual("PreLegalize.out");

    mgr.banking();
    mgr.getOverallCost(cost_verbose);
    mgr.dumpVisual("Banking.out");

    mgr.postBankingOptimize();
    mgr.getOverallCost(cost_verbose);
    mgr.dumpVisual("PostCG.out");

    mgr.legalize();
    mgr.getOverallCost(cost_verbose);
    mgr.dumpVisual("Legalize.out");
    mgr.checker();

    mgr.detailplacement();
    mgr.getOverallCost(cost_verbose);
    mgr.dumpVisual("DetailPlacement.out");
    mgr.checker();

    mgr.dump(argv[2]);
    return 0;
}
