#ifndef _FF_H_
#define _FF_H_

#include "Cell.h"

class Cell;

class FF{
private:
    string instanceName;
    string cellName;
    Coor coor;
    Cell cell;
    unordered_map<string, double> TimingSlack;

    // for find optimal location
    Cell* prev_cell;
    FF* prev_ff;

    int idx;
public:
    FF();
    ~FF();

    // setter
    void setInstanceName(const string &);
    void setCellName(const string &);
    void setCoor(Coor &);
    void setCell(const Cell &);
    void setTimingSlack(double, const string&);
    void setIdx(int i){this->idx = i;}
    // getter
    const string &getInstanceName()const;
    const string &getCellName()const;
    const Cell& getCell()const;
    int getBits()const;
    double getW()const;
    double getH()const;
    Coor getCoor(){return this->coor;}
    int getIdx(){return this->idx;}
    int getPinCount()const;
    const Coor &getPinCoor(const string &)const;
    double getTimingSlack(const string&);

    friend ostream &operator<<(ostream &, const FF &);
};


#endif
