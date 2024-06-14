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

void Cell::addPinName(const string &pinName){
    this->pinName.push_back(pinName);
}

void Cell::setQpinDelay(double QpinDelay){
    this->QpinDelay = QpinDelay;
}

void Cell::setGatePower(double GatePower){
    this->GatePower = GatePower;
}

// getter
const string &Cell::getCellName()const{
    return cellName;
}

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

double Cell::getQpinDelay(){
    assert(isFF);
    return QpinDelay;
}

double Cell::getGatePower(){
    assert(isFF);
    return GatePower;
}

ostream &operator<<(ostream &out, const Cell &c){
    if(c.isFF){
        out << "CellName: " << c.cellName << endl;
        out << "IsFF: " << c.isFF << endl;
        out << "Number of bits: " << c.bits << endl;
        out << "Width: " << c.w << endl;
        out << "Height: " << c.h << endl;
        out << "Pin Count: " << c.pinCount << endl;
        out << "QpinDelay: " << c.QpinDelay << endl;
        out << "GatePower: " << c.GatePower << endl;
    }
    else{
        out << "CellName: " << c.cellName << endl;
        out << "IsFF: " << c.isFF << endl;
        out << "Width: " << c.w << endl;
        out << "Height: " << c.h << endl;
        out << "Pin Count: " << c.pinCount << endl;
    }

    for(const auto &pair : c.pinCoorMap){
        out << "\t" << pair.first << " " << pair.second << endl;
    }
    return out;
}