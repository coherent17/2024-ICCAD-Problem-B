#ifndef _MANAGER_H_
#define _MANAGER_H_

#include <iostream>
#include <fstream>
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
#include "MeanShift.h"
#include "Preprocess.h"
#include "Cluster.h"
#include "Banking.h"
#include "Legalizer.h"
#include "Util.h"

class FF;
class Preprocess;
class Cluster;
class Banking;
class Param;
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
    std::unordered_map<int, std::vector<Cell *>> Bit_FF_Map;
    int MaxBit;

    // Instance
    int NumInstances;
    std::unordered_map<std::string, FF *> FF_Map;
    std::unordered_map<std::string, Gate *> Gate_Map;
    

    // Netlist
    int NumNets;
    std::unordered_map<std::string, Net> Net_Map;

    // for naming
    std::unordered_map<std::string, int> name_record;

    // preprocess
    Preprocess* preprocessor;

    // cluster
    std::vector<Cluster> clusters;

    // parameter
    Param param;

    // ######################################### used in cluster ########################################################
    // all should be single bit ff in here.    
    std::vector<FF *> FFs;
    // ######################################### used in cluster ########################################################

public:
    Manager();
    ~Manager();

    void parse(const std::string &filename);
    void preprocess();
    void meanshift();
    void banking();
    void legalize();
    void dump(const std::string &filename);
    void dumpVisual(const std::string &filename);
    void print();
    
    std::string getNewFFName(const std::string&); // using a prefix string to get new unique FF name
                                                  // suggest prefix for N-bit MBFF -> FF_N_

    FF* bankFF(Coor newbankCoor, Cell* bankCellType, std::vector<FF*> FFToBank);
    // given newbankCoor (left down) and target celltype
    // it will bank all the FF in vector (can be MBFF in FFToBank)
    // and it will delete old and insert new FF to FF_Map
    std::vector<FF*> debankFF(FF* MBFF, Cell* debankCellType);
    // the FF after debank will be assign to debankCellType (maybe this can be a vector)
    void getNS(double& TNS, double& WNS);

    friend class Parser;
    friend class Dumper;
    friend class MeanShift;
    friend class Preprocess;
    friend class Banking;
    friend class Legalize;

private:
    bool isIOPin(const std::string &pinName);
};

#endif