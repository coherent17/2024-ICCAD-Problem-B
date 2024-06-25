#ifndef _MANAGER_H_
#define _MANAGER_H_

#include <iostream>
#include <string>
#include <unordered_map>
#include "Coor.h"
#include "Instance.h"
#include "Die.h"
#include "Cell_Library.h"
#include "FF.h"
#include "Gate.h"
#include "Net.h"
#include "Parser.h"
#include "Dumper.h"

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
    std::unordered_map<std::string, Coor> Input_Map;
    std::unordered_map<std::string, Coor> Output_Map;
    std::unordered_map<std::string, Instance> IO_Map;

    // Cell library
    Cell_Library cell_library;

    // Instance
    int NumInstances;
    std::unordered_map<std::string, FF> FF_Map;
    std::unordered_map<std::string, Gate> Gate_Map;

    // Netlist
    int NumNets;
    std::unordered_map<std::string, Net> Net_Map;


    // For debanking
    // std::unordered_map<std::string, FF> FF_list; // list of all FF, in one bit without physical info.
    // std::unordered_map<std::string, std::string> FF_list_Map; // map input MBFF to FF_list, MBFF_NAME/PIN_NAME -> FF_list key




public:
    Manager();
    ~Manager();

    void parse(const std::string &filename);
    // void Build_Logic_FF();
    // void Build_Circuit_Gragh();
    // void optimal_FF_location();
    
    void meanshift();
    void dump(const std::string &filename);
    void print();
    friend class Parser;
    friend class Dumper;

private:
    bool isIOPin(const std::string &pinName);
};

#endif