#ifndef _MANAGER_H_
#define _MANAGER_H_

#include "Util.h"
#include "Cell_Library.h"
#include "FF.h"
#include "Gate.h"
#include "Parser.h"
#include "Net.h"
#include "Die.h"

class Manager{
public:
    // cost function weight
    double alpha;
    double beta;
    double gamma;
    double lambda;
    double DisplacementDelay;
    
    // die info
    Die die;

    // I/O pin coordinate
    int NumInput;
    int NumOutput;
    unordered_map<string, Coor> Input_Map;
    unordered_map<string, Coor> Output_Map;
    unordered_map<string, Instance> IO_Map;

    // Cell library
    Cell_Library cell_library;

    // Instance
    int NumInstances;
    unordered_map<string, FF> FF_Map;
    unordered_map<string, Gate> Gate_Map;
    unordered_map<string, FF> FF_list; // list of all FF, in one bit without physical info.
    unordered_map<string, string> FF_list_Map; // map input MBFF to FF_list, MBFF_NAME/PIN_NAME -> FF_list key

    // Netlist
    int NumNets;
    unordered_map<string, Net> Net_Map;


public:
    Manager();
    ~Manager();

    void Read_InputFile(const string &);
    void Technology_Mapping();
    void Build_Logic_FF();
    void Build_Circuit_Gragh();
    void optimal_FF_location();
    void print();
    friend class Parser;

private:
    bool isIOPin(const string &);
};

#endif