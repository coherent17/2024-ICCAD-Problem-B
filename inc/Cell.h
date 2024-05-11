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
    void setQpinDelay(double);
    void setGatePower(double);

    // getter
    const string &getCellName()const;
    bool getisFF()const;
    int getBit()const;
    double getW()const;
    double getH()const;
    int getPinCount()const;
    const Coor &getPinCoor(const string &)const;
    double getQpinDelay();
    double getGatePower();

    friend ostream &operator<<(ostream &, const Cell &);
};

#endif