#ifndef _GATE_H_
#define _GATE_H_

#include "Cell.h"

class Gate{
private:
    string instanceName;
    string cellName;
    Coor coor;
    Cell *cellLibraryPtr;

public:
    Gate();
    ~Gate();

    // setter
    void setInstanceName(const string &);
    void setCellName(const string &);
    void setCoor(Coor &);
    void setCellLibraryPtr(Cell *);

    // getter
    double getW()const;
    double getH()const;
    int getPinCount()const;
    const Coor &getPinCoor(const string &)const;

    friend ostream &operator<<(ostream &, const Gate &);
};

#endif