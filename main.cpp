#include "Manager.h"

int main(int argc, char *argv[]){
    Manager mgr;
    mgr.Read_InputFile(argv[1]);
    mgr.Technology_Mapping();
    mgr.print();
    // mgr.preprocess();
    // mgr.optimal_FF_location();
    return 0;
}