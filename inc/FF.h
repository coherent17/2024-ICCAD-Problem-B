#ifndef _FF_H_
#define _FF_H_

#include "Cell.h"

class FF{
private:
    string instanceName;
    string cellName;
    Coor coor;
    Cell *cellLibraryPtr;
    double TimingSlack;

public:
    FF();
    ~FF();

    // setter
    void setInstanceName(const string &);
    void setCellName(const string &);
    void setCoor(Coor &);
    void setCellLibraryPtr(Cell *);
    void setTimingSlack(double);

    // getter
    int getBits()const;
    double getW()const;
    double getH()const;
    int getPinCount()const;
    const Coor &getPinCoor(const string &)const;
    double getTimingSlack()const;

    friend ostream &operator<<(ostream &, const FF &);
};

#endif