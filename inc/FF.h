#ifndef _FF_H_
#define _FF_H_

#include "Cell.h"

class Cell;

class FF : public Instance{
private:
    unordered_map<string, double> TimingSlack;


    // for circuit gragh
    vector<string> inputIO; // contain all input cells/FFs/IOs
    vector<string> outputIO; // contain all output cells/FFs/IOs
    // for find optimal location
    Cell* prev_cell;
    FF* prev_ff;

    int idx;
public:
    FF();
    ~FF();

    // setter
    void setTimingSlack(double, const string&);
    void setIdx(int i){this->idx = i;}
    // getter
    int getIdx(){return this->idx;}
    double getTimingSlack(const string&);

    friend ostream &operator<<(ostream &, const FF &);
};


#endif
