#ifndef _GATE_H_
#define _GATE_H_

#include "Cell.h"

class Gate{
private:
    string instanceName;
    string cellName;
    Coor coor;
    Cell cell;

public:
    Gate();
    ~Gate();

    // setter
    void setInstanceName(const string &);
    void setCellName(const string &);
    void setCoor(Coor &);
    void setCell(const Cell &);

    // getter
    const string &getInstanceName()const;
    const string &getCellName()const;
    double getW()const;
    double getH()const;
    int getPinCount()const;
    const Coor &getPinCoor(const string &)const;

    friend ostream &operator<<(ostream &, const Gate &);
};

#endif