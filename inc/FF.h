#ifndef _FF_H_
#define _FF_H_

#include <iostream>
#include <string>
#include <unordered_map>
#include "Instance.h"

class FF : public Instance{
private:
    std::unordered_map<std::string, double> TimingSlack;

    // // for find optimal location
    // Cell* prev_cell;
    // FF* prev_ff;
    // int idx;

public:
    FF();
    ~FF();

    // Setters
    void setTimingSlack(const std::string &pinName, double slack);
    //void setIdx(int i){this->idx = i;}
    
    // Getter
    double getTimingSlack(const std::string &pinName)const;
    //int getIdx(){return this->idx;}
    

    friend std::ostream &operator<<(std::ostream &os, const FF &ff);
};


#endif
