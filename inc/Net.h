#ifndef _NET_H_
#define _NET_H_

#include "Util.h"
#include "Pin.h"
using namespace std;

class Net{
private:
    string netName;
    int numPins;
    vector<Pin> pins;

public:
    Net();
    ~Net();

    // setter
    void setNetName(const string &);
    void setNumPins(int);
    void addPins(const Pin &);

    // getter
    const string &getNetName()const;
    int getNumPins()const;
    const Pin& getPin(int)const;
    
    friend ostream &operator<<(ostream &, const Net &);
};

#endif