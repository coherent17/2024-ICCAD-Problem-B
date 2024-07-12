#include "Manager.h"
#include <iostream>
#include <vector>

int main(int argc, char *argv[]){

    if (argc < 2){
        std::cerr << "Usage: " << argv[0] << " <inputfile> <outputfile>" << std::endl;
        return EXIT_FAILURE;
    }

    Manager mgr;
    mgr.parse(argv[1]);
    mgr.preprocess();

    mgr.meanshift();
    // mgr.print();
    mgr.dump(argv[2]);
    
    mgr.banking();
    if(argc == 4){
        mgr.dumpVisual(argv[3]);
    }

    // place holder method for runing legalize algorithm
    // mgr.legalize();
    return 0;
}