#include "Manager.h"

int main(int argc, char *argv[]){
    Manager mgr;
    mgr.parse(argv[1]);
    //mgr.print();
    mgr.dump(argv[2]);
    // mgr.Technology_Mapping();
    // mgr.Build_Logic_FF();
    // mgr.Build_Circuit_Gragh();
    
    // mgr.preprocess();
    // mgr.optimal_FF_location();
    return 0;
}