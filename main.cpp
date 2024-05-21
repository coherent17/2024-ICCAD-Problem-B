#include "Manager.h"

int main(int argc, char *argv[]){
    Manager mgr;
    mgr.Read_InputFile(argv[1]);
    mgr.Technology_Mapping();
    mgr.print();
    optimal_FF_location(mgr);
    return 0;
}