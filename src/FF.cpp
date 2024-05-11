#include "FF.h"

FF::FF(){
    cellLibraryPtr = nullptr;
}

FF::~FF(){
    ;
}

// setter
void FF::setInstanceName(const string &instanceName){
    this->instanceName = instanceName;
}

void FF::setCellName(const string &cellName){
    this->cellName = cellName;
}

void FF::setCoor(Coor &coor){
    this->coor = coor;
}

void FF::setCellLibraryPtr(Cell *cell){
    this->cellLibraryPtr = cell;
}

void FF::setTimingSlack(double TimingSlack){
    this->TimingSlack = TimingSlack;
}

// getter
int FF::getBits()const{
    assert(cellLibraryPtr != nullptr);
    return cellLibraryPtr->getBit();
}

double FF::getW()const{
    assert(cellLibraryPtr != nullptr);
    return cellLibraryPtr->getW();
}

double FF::getH()const{
    assert(cellLibraryPtr != nullptr);
    return cellLibraryPtr->getH();
}

int FF::getPinCount()const{
    assert(cellLibraryPtr != nullptr);
    return cellLibraryPtr->getPinCount();
}

const Coor &FF::getPinCoor(const string &pinName)const{
    assert(cellLibraryPtr != nullptr);
    return cellLibraryPtr->getPinCoor(pinName);
}

double FF::getTimingSlack()const{
    return TimingSlack;
}

ostream &operator<<(ostream &out, const FF &ff){
    out << "Instance Name: " << ff.instanceName << endl;
    out << "Coor: " << ff.coor << endl;
    out << "TimingSlack: " << ff.TimingSlack << endl;
    out << "CellType: " << ff.cellLibraryPtr->getCellName() << endl;
    return out;
}
