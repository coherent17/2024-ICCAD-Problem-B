#ifndef _CELL_H_
#define _CELL_H_

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cassert>
#include "Coor.h"

enum class Cell_Type{
    UNSET = 0,
    FF = 1,
    Gate = 2
};

// The cell will be used in cell_library
class Cell{
private:
    std::string cellName;
    Cell_Type type;
    int bits;
    double w;
    double h;
    int pinCount;
    std::vector<std::string> pinNames;
    std::unordered_map<std::string, Coor> pinCoorMap;
    double QpinDelay;
    double GatePower;

public:
    Cell();
    virtual ~Cell();

    // Setters
    void setCellName(const std::string &cellName);
    void setType(Cell_Type type);
    void setBits(int bits);
    void setW(double w);
    void setH(double h);
    void setPinCount(int pinCount);
    void addPinCoor(const std::string &pinName, const Coor &coor);
    void addPinName(const std::string &pinName);
    void setQpinDelay(double delay);
    void setGatePower(double power);

    // Getters
    const std::string &getCellName()const;
    Cell_Type getType()const;
    int getBits()const;
    double getW()const;
    double getH()const;
    int getPinCount()const;
    const std::string &getPinName(size_t pinIdx)const;
    const Coor &getPinCoor(const std::string &pinName)const;
    std::unordered_map<std::string, Coor> getPinCoorMap()const;
    double getQpinDelay() const;
    double getGatePower() const;

    // Stream Insertion
    friend std::ostream &operator<<(std::ostream &os, const Cell &cell);
};

#endif