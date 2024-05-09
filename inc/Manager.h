#ifndef _MANAGER_H_
#define _MANAGER_H_

#include "Cell_Library.h"
#include "FF.h"
#include "Gate.h"
#include "Parser.h"

class Manager{
private:
    // cost function weight
    double alpha;
    double beta;
    double gamma;
    double lambda;
    
    // die coordinate
    Coor DieOrigin;
    Coor DieBorder;

    // I/O pin coordinate
    int NumInput;
    int NumOutput;
    unordered_map<string, Coor> Input_Map;
    unordered_map<string, Coor> Output_Map;

    Cell_Library cell_library;
    unordered_map<string, FF> FF_Map;
    unordered_map<string, Gate> Gate_Map;

public:
    Manager();
    ~Manager();

    void Read_InputFile(const string &);
    void print();
    friend class Parser;
};

#endif