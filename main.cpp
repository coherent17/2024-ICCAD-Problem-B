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
    std::cout << "Slack after preprocess" << std::endl;
    mgr.showNS();

    mgr.meanshift();
    std::cout << "Slack after meanshift" << std::endl;
    mgr.showNS();
    // mgr.print();
    mgr.dump(argv[2]);
    
    mgr.banking();
    std::cout << "Slack after banking" << std::endl;
    mgr.showNS();
    if(argc == 4){
        mgr.dumpVisual(argv[3]);
    }

    // place holder method for runing legalize algorithm
    mgr.legalize();
    return 0;
}