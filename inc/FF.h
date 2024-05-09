#ifndef _FF_H_
#define _FF_H_

#include "Cell.h"

class FF{
private:
    string instanceName;
    string cellName;
    Coor coor;
    Cell *cellLibraryPtr;
    double QpinDelay;
    double TimingSlack;
    double GatePower;

public:
    FF();
    ~FF();

    // setter
    void setInstanceName(const string &);
    void setCellName(const string &);
    void setCoor(Coor &);
    void setCellLibraryPtr(Cell *);

    // getter
    int getBits()const;
    double getW()const;
    double getH()const;
    int getPinCount()const;
    const Coor &getPinCoor(const string &)const;

    friend ostream &operator<<(ostream &, const FF &);
};

#endif