#include "Cell.h"

// constructor & destructor
Cell::Cell(){
    this->bits = 0;
    this->cellName = "";
    this->isFF = false;
    this->w = 0;
    this->h = 0;
    this->pinCount = 0;
}

Cell::~Cell(){
    ;
}

// setter
void Cell::setCellName(const string &cellName){
    this->cellName = cellName;
}

void Cell::setIsFF(bool isFF){
    this->isFF = isFF;
}

void Cell::setBits(int bits){
    this->bits = bits;
}

void Cell::setW(double w){
    this->w = w;
}

void Cell::setH(double h){
    this->h = h;
}

void Cell::setPinCount(int pinCount){
    this->pinCount = pinCount;
}

void Cell::addPinCoor(const string &pinName, Coor &coor){
    this->pinCoorMap[pinName] = coor;
}

// getter
bool Cell::getisFF()const{
    return isFF;
}

int Cell::getBit()const{
    return bits;
}

double Cell::getW()const{
    return w;
}

double Cell::getH()const{
    return h;
}

int Cell::getPinCount()const{
    return pinCount;
}

const Coor &Cell::getPinCoor(const string &pinName)const{
    auto it = pinCoorMap.find(pinName);
    assert(it != pinCoorMap.end());
    return it->second;
}

ostream &operator<<(ostream &out, const Cell &c){
    if(c.isFF){
        out << "FlipFlop:" << endl;
        out << "\tNumber of bits: " << c.bits << endl;
        out << "\tWidth: " << c.w << endl;
        out << "\tHeight: " << c.h << endl;
        out << "\tPin Count: " << c.pinCount << endl;
    }
    else{
        out << "Gate:" << endl;
        out << "\tWidth: " << c.w << endl;
        out << "\tHeight: " << c.h << endl;
        out << "\tPin Count: " << c.pinCount << endl;
    }

    for(const auto &pair : c.pinCoorMap){
        out << "\t\t" << pair.first << " " << pair.second << endl;
    }
    return out;
}