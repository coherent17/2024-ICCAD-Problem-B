#include "Gate.h"

Gate::Gate(){
    ;
}

Gate::~Gate(){
    ;
}

// setter
void Gate::setInstanceName(const string &instanceName){
    this->instanceName = instanceName;
}

void Gate::setCellName(const string &cellName){
    this->cellName = cellName;
}

void Gate::setCoor(Coor &coor){
    this->coor = coor;
}

void Gate::setCell(const Cell &cell){
    this->cell = cell;
}

// getter
const string &Gate::getInstanceName()const{
    return instanceName;
}

const string &Gate::getCellName()const{
    return cellName;
}

double Gate::getW()const{
    return cell.getW();
}

double Gate::getH()const{
    return cell.getH();
}


int Gate::getPinCount()const{
    return cell.getPinCount();
}

const Coor &Gate::getPinCoor(const string &pinName)const{
    return cell.getPinCoor(pinName);
}

ostream &operator<<(ostream &out, const Gate &gate){
    out << "Instance Name: " << gate.instanceName << endl;
    out << "Coor: " << gate.coor << endl;
    out << gate.cell << endl;
    return out;
}