#include "Manager.h"
#include <iostream>
#include <vector>

int main(int argc, char *argv[]){

    if (argc < 2){
        std::cerr << "Usage: ./" << argv[0] << " <inputfile> <outputfile>" << std::endl;
        return EXIT_FAILURE;
    }

    Manager mgr;
    mgr.parse(argv[1]);
    mgr.Debank();
    mgr.Build_Circuit_Gragh();
    mgr.optimal_FF_location();

    // mgr.meanshift();
    // mgr.print();
    // mgr.dump(argv[2]);
    // if(argc == 4){
    //     mgr.dumpVisual(argv[3]);
    // }


    // mgr.Build_Circuit_Gragh();
    
    // mgr.preprocess();
    // mgr.optimal_FF_location();
    return 0;
}