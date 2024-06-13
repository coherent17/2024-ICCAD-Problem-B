#include "FF.h"

FF::FF(){
    ;
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

void FF::setCell(const Cell &cell){
    this->cell = cell;
}

void FF::setTimingSlack(double TimingSlack){
    this->TimingSlack = TimingSlack;
}

// getter
const string &FF::getInstanceName()const{
    return instanceName;
}

const string &FF::getCellName()const{
    return cellName;
}

const Cell& FF::getCell()const{
    return cell;
}

int FF::getBits()const{
    return cell.getBit();
}

double FF::getW()const{
    return cell.getW();
}

double FF::getH()const{
    return cell.getH();
}

int FF::getPinCount()const{
    return cell.getPinCount();
}

const Coor &FF::getPinCoor(const string &pinName)const{
    return cell.getPinCoor(pinName);
}

double FF::getTimingSlack()const{
    return TimingSlack;
}

ostream &operator<<(ostream &out, const FF &ff){
    out << "Instance Name: " << ff.instanceName << endl;
    out << "Coor: " << ff.coor << endl;
    out << "TimingSlack: " << ff.TimingSlack << endl;
    out << ff.cell << endl;
    return out;
}
