#ifndef _CELL_H_
#define _CELL_H_

#include "Util.h"

class Cell{
private:
    string cellName;
    bool isFF;
    int bits;
    double w;
    double h;
    int pinCount;
    vector<string> pinName;
    unordered_map<string, Coor> pinCoorMap;
    double QpinDelay;
    double GatePower;

public:
    Cell();
    ~Cell();

    // setter
    void setCellName(const string &);
    void setIsFF(bool);
    void setBits(int);
    void setW(double);
    void setH(double);
    void setPinCount(int);
    void addPinCoor(const string &, Coor &);
    void addPinName(const string &);
    void setQpinDelay(double);
    void setGatePower(double);

    // getter
    const string &getCellName()const;
    bool getisFF()const;
    int getBit()const;
    double getW()const;
    double getH()const;
    int getPinCount()const;
    const string &getPinName(const int&)const;
    const Coor &getPinCoor(const string &)const;
    double getQpinDelay();
    double getGatePower();

    friend ostream &operator<<(ostream &, const Cell &);
};

class Instance{
protected:
    string instanceName;
    string cellName;
    Coor coor;
    Cell cell;

    // for circuit gragh
    vector<string> inputIO; // contain all input cells/FFs/IOs
    vector<string> outputIO; // contain all output cells/FFs/IOs

public:
    Instance();
    ~Instance();

    // setter
    void setInstanceName(const string &);
    void setCellName(const string &);
    void setCoor(Coor &);
    void setCell(const Cell &);

    // getter
    const string &getInstanceName()const;
    const string &getCellName()const;
    const Cell& getCell()const;
    double getW()const;
    double getH()const;
    Coor getCoor()const;
    int getPinCount()const;
    const Coor &getPinCoor(const string &)const;
};

#endif