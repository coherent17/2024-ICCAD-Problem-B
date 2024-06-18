#include "FF.h"

FF::FF(){
    ;
}

FF::~FF(){
    ;
}

// setter

void FF::setTimingSlack(double TimingSlack, const string& pinName){
    this->TimingSlack[pinName] = TimingSlack;
}

// getter
double FF::getTimingSlack(const string& pinName){
    return TimingSlack[pinName];
}

ostream &operator<<(ostream &out, const FF &ff){
    out << "Instance Name: " << ff.instanceName << endl;
    out << "Coor: " << ff.coor << endl;
    for(auto& s : ff.TimingSlack)
        out << "Pin: " << s.first << "TimingSlack: " << s.second << endl;
    out << ff.cell << endl;
    return out;
}
