#include "Gate.h"

Gate::Gate(){}

Gate::~Gate(){}

std::ostream &operator<<(std::ostream &os, const Gate &gate){
    os << "Instance Name: " << gate.instanceName << std::endl;
    os << "Coor: " << gate.coor << std::endl;
    os << "CellName: " << gate.getCell()->getCellName() << std::endl;
    return os;
}