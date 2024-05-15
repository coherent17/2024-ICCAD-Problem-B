#ifndef _NET_H_
#define _NET_H_

#include "Util.h"

using namespace std;

class Net{
private:
    string netName;
    int numPins;
    unordered_set<string> pins;

public:
    Net();
    ~Net();

};

#endif