#ifndef _MANAGER_H_
#define _MANAGER_H_

#include <iostream>
#include <cstdlib>
#include <cstdio>
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
#include "DetailPlacement.h"
#include "Util.h"
#include "PrettyTable.h"
#include "PostBankingOptimizer.h"
#include "Checker.h"

#ifdef ENABLE_DEBUG_MGR
#define DEBUG_MGR(message) std::cout << "[MANAGER] " << message << std::endl
#else
#define DEBUG_MGR(message)
#endif

class FF;
class Preprocess;
class Cluster;
class Banking;
class Param;
class Legalizer;
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
    std::unordered_map<std::string, FF *> originalFF_Map;
    std::unordered_map<std::string, Gate *> Gate_Map;
    

    // Netlist
    int NumNets;
    std::unordered_map<std::string, Net> Net_Map;

    // for naming
    std::unordered_map<std::string, int> name_record;

    // preprocess
    Preprocess* preprocessor;

    // Legalize
    Legalizer* legalizer;

    // parameter
    Param param;

    // pointer recycle
    std::queue<FF*> FFGarbageCollector;

    // IO filename
    std::string input_filename;

public:
    Manager();
    ~Manager();

    void parse(const std::string &filename);
    void preprocess();
    void meanshift();
    void preLegalize();
    void banking();
    void postBankingOptimize();
    void legalize();
    void detailplacement();
    void checker();

    void dump(const std::string &filename);
    void dumpVisual(const std::string &filename);
    void print();
    
    std::string getNewFFName(const std::string&); // using a prefix string to get new unique FF name
                                                  // suggest prefix for N-bit MBFF -> FF_N_

    FF* bankFF(Coor newbankCoor, Cell* bankCellType, std::vector<FF*> FFToBank);
    // given newbankCoor (left down) and target celltype
    // it will bank all the FF in vector (can be MBFF in FFToBank)
    // and it will delete old and insert new FF to FF_Map
    void assignSlot(FF* newFF);
    std::vector<FF*> debankFF(FF* MBFF, Cell* debankCellType);
    // the FF after debank will be assign to debankCellType (maybe this can be a vector)
    void getNS(double& TNS, double& WNS, bool show); // this retunr TNS and WNS of whole design (all FF in FF_Map)
    double getTNS();
    double getWNS();
    void showNS();
    // for lib cell scoring
    void libScoring();
    static void sortCell(std::vector<Cell *> &cell_vector);
    
    // pointer recycle
    FF* getNewFF();
    void deleteFF(FF*);
    
    double getCostDiff(Coor newbankCoor, Cell* bankCellType, std::vector<FF*>& FFToBank); // > 0 -> after bank cost will be larger
    double getEvaluatorCost();
    double calculateBinDensityCost();
    double getOverallCost(bool verbose, bool runEvaluator);
    friend class Parser;
    friend class Dumper;
    friend class MeanShift;
    friend class Preprocess;
    friend class Banking;
    friend class Legalizer;
    friend class DetailPlacement;
    friend class Checker;

private:
    bool isIOPin(const std::string &pinName);
};

#endif