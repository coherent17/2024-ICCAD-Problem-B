#ifndef _CELL_H_
#define _CELL_H_

#include "Util.h"

class Cell{
private:
    bool isFF;
    int bits;
    double w;
    double h;
    int pinCount;
    unordered_map<string, Coor> pinCoorMap;

public:
    Cell();
    ~Cell();

    bool getisFF()const;
    int getBit()const;
    double getW()const;
    double getH()const;
    int getPinCount()const;
    Coor &getPinCoor()const;
};

#endif