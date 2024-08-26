#include "Gate.h"

Gate::Gate() : visitedTime(0),  maxInput({nullptr, nullptr, "", 0}){
}

Gate::~Gate(){}

// setters
void Gate::updateVisitedTime(){
    this->visitedTime++;
}

void Gate::setMaxInput(MaxInput maxInput){
    if(maxInput.cost > this->maxInput.cost){
        this->maxInput = maxInput;
    }
}

// getters 
int Gate::getVisitedTime(){
    return this->visitedTime;
}

MaxInput Gate::getMaxInput(){
    return this->maxInput;
}

std::ostream &operator<<(std::ostream &os, const Gate &gate){
    os << "Instance Name: " << gate.instanceName << std::endl;
    os << "Coor: " << gate.coor << std::endl;
    os << "CellName: " << gate.getCell()->getCellName() << std::endl;
    return os;
}