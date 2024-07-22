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
    Coor coor;
    const Cell *cell;

    // for circuit gragh
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> inputInstances; // cur input pin name -> {input instance name, input instance output pin}
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> outputInstances; // cur output pin name -> {output instance name and its input pin name}

public:
    Instance();
    virtual ~Instance();

    // Setters
    void setInstanceName(const std::string &instanceName);
    void setCoor(const Coor &coor);
    void setCell(const Cell *cell);
    void addInput(const std::string& pinName, const std::string &input, const std::string& instPinName);
    void addOutput(const std::string& pinName, const std::string &output, const std::string& instPinName);

    // Getters
    const std::string &getInstanceName()const;
    Coor getCoor()const;
    const Cell *getCell()const;
    double getW()const;
    double getH()const;
    int getPinCount()const;
    const Coor &getPinCoor(const std::string &pinName)const;
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>>& getInputInstances();
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>>& getOutputInstances();
};

#endif