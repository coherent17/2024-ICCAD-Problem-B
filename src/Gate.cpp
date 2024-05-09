#include "Gate.h"

Gate::Gate(){
    cellLibraryPtr = nullptr;
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

void Gate::setCellLibraryPtr(Cell *cell){
    this->cellLibraryPtr = cell;
}

// getter
double Gate::getW()const{
    assert(cellLibraryPtr != nullptr);
    return cellLibraryPtr->getW();
}

double Gate::getH()const{
    assert(cellLibraryPtr != nullptr);
    return cellLibraryPtr->getH();
}


int Gate::getPinCount()const{
    assert(cellLibraryPtr != nullptr);
    return cellLibraryPtr->getPinCount();
}

const Coor &Gate::getPinCoor(const string &pinName)const{
    assert(cellLibraryPtr != nullptr);
    return cellLibraryPtr->getPinCoor(pinName);
}

ostream &operator<<(ostream &out, const Gate &gate){
    out << "Instance Name: " << gate.instanceName << endl;
    out << "Coor: " << gate.coor << endl;
    out << *gate.cellLibraryPtr << endl;
    return out;
}