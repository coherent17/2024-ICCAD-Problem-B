#ifndef _INSTANCE_H_
#define _INSTANCE_H_

#include <iostream>
#include <string>
#include <vector>
#include "Coor.h"
#include "Cell.h"

class Instance{
protected:
    std::string instanceName;
    std::string cellName;
    Coor coor;
    const Cell *cell;

    // the largest neightbor instance(critical path)
    Instance* largestInput; // IO pin or FF in FF_list
    std::pair<Instance*, std::string> largestOutput;

    // for circuit gragh
    std::vector<std::string> inputInstances; // contain all input cells/FFs/IOs
    std::vector<std::pair<std::string, std::string>> outputInstances; // contain all output cells/FFs/IOs, pair of (instance name, pin name)

public:
    Instance();
    virtual ~Instance();

    // Setters
    void setInstanceName(const std::string &instanceName);
    void setCellName(const std::string &cellName);
    void setCoor(const Coor &coor);
    void setCell(const Cell *cell);
    void setLargestInput(Instance *input);
    void setLargestOutput(Instance *output, const std::string& pinName);
    void addInput(const std::string &input);
    void addOutput(const std::string &output, const std::string& pinName);

    // Getters
    const std::string &getInstanceName()const;
    const std::string &getCellName()const;
    Coor getCoor()const;
    const Cell *getCell()const;
    double getW()const;
    double getH()const;
    int getPinCount()const;
    const Coor &getPinCoor(const std::string &pinName)const;
    const Instance* getLargestInput() const;
    const std::pair<Instance*, std::string>getLargestOutput() const;
    const std::vector<std::string> &getInputInstances() const;
    const std::vector<std::pair<std::string, std::string>> &getOutputInstances() const;
};

#endif