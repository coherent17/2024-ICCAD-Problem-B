#ifndef _NET_H_
#define _NET_H_

#include <iostream>
#include <unordered_set>
#include <string>

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