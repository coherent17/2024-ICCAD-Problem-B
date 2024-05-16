#ifndef _FF_H_
#define _FF_H_

#include "Cell.h"

class FF{
private:
    string instanceName;
    string cellName;
    Coor coor;
    Cell cell;
    double TimingSlack;

public:
    FF();
    ~FF();

    // setter
    void setInstanceName(const string &);
    void setCellName(const string &);
    void setCoor(Coor &);
    void setCell(const Cell &);
    void setTimingSlack(double);

    // getter
    const string &getInstanceName()const;
    const string &getCellName()const;
    int getBits()const;
    double getW()const;
    double getH()const;
    int getPinCount()const;
    const Coor &getPinCoor(const string &)const;
    double getTimingSlack()const;

    friend ostream &operator<<(ostream &, const FF &);
};

#endif