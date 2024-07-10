#ifndef _GATE_H_
#define _GATE_H_

#include <iostream>
#include "Instance.h"

class Gate;
struct MaxInput
{
    Instance* instance; // start point of critical path INPUT/FF
    Gate* outputGate; // 
    std::string pinName; // input pin of outputGate
    double cost; // QpinDelay + accumulate HPWL
};

class Gate : public Instance{
private:
    // for delay propagation
    int visitedTime;
    MaxInput maxInput; // Max input and its output cell's input pin from propagation

public:
    Gate();
    ~Gate();
    // setters
    void updateVisitedTime();
    void setMaxInput(MaxInput maxInput);

    // getters
    int getVisitedTime();
    MaxInput getMaxInput();

    friend std::ostream &operator<<(std::ostream &os, const Gate &gate);
};

#endif
