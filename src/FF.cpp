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

void FF::setTimingSlack(double TimingSlack, const string& pinName){
    this->TimingSlack[pinName] = TimingSlack;
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

double FF::getTimingSlack(const string& pinName){
    return TimingSlack[pinName];
}

ostream &operator<<(ostream &out, const FF &ff){
    out << "Instance Name: " << ff.instanceName << endl;
    out << "Coor: " << ff.coor << endl;
    for(auto& s : ff.TimingSlack)
        out << "Pin: " << s.first << "TimingSlack: " << s.second << endl;
    out << ff.cell << endl;
    return out;
}


//-------------------------Logic FF----------------------//
logicFF::logicFF(){

}

logicFF::~logicFF(){

}

void logicFF::setInstanceName(const string& s){
    this->instanceName = s;
}

void logicFF::setCoor(Coor &c){
    this->coor = c;
}

void logicFF::setTimingSlack(double t){
    this->TimingSlack = t;
}

const string &logicFF::getInstanceName()const{
    return this->instanceName;
}

const Coor& logicFF::getCoor()const{
    return this->coor;
} 

double logicFF::getTimingSlack()const{
    return this->TimingSlack;
}