#ifndef _NET_H_
#define _NET_H_

#include <iostream>
#include <string>
#include <vector>
#include "Pin.h"

class Net{
private:
    std::string netName;
    int numPins;
    std::vector<Pin> pins;

public:
    Net();
    ~Net();

    // Setters
    void setNetName(const std::string &netName);
    void setNumPins(int numPins);
    void addPins(const Pin &pin);

    // Getters
    const std::string &getNetName()const;
    int getNumPins()const;
    const Pin &getPin(int pinIdx)const;
    
    friend std::ostream &operator<<(std::ostream &os, const Net &net);
};

#endif