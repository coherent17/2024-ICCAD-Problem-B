#include "Cell.h"

// constructor & destructor
Cell::Cell() : 
    cellName(""), 
    type(Cell_Type::UNSET), 
    bits(0), 
    w(0), 
    h(0), 
    score(0),
    pinCount(0), 
    inputCount(0),
    QpinDelay(0), 
    GatePower(0){}

Cell::~Cell(){}

// Setters
void Cell::setCellName(const std::string &cellName){
    this->cellName = cellName;
}

void Cell::setType(Cell_Type type){
    this->type = type;
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

void Cell::setScore(double score){
    this->score = score;
}

void Cell::setPinCount(int pinCount){
    this->pinCount = pinCount;
}

void Cell::setInputCount(int inputCount){
    this->inputCount = inputCount;
}

void Cell::addPinCoor(const std::string &pinName, const Coor &coor){
    this->pinCoorMap[pinName] = coor;
}

void Cell::addPinName(const std::string &pinName){
    this->pinNames.push_back(pinName);
}

void Cell::setQpinDelay(double delay){
    this->QpinDelay = delay;
}


void Cell::setGatePower(double power){
    this->GatePower = power;
}

// Getters
const std::string &Cell::getCellName()const{
    return cellName;
}

Cell_Type Cell::getType() const {
    return type;
}

int Cell::getBits()const{
    return bits;
}

double Cell::getW()const{
    return w;
}

double Cell::getH()const{
    return h;
}

double Cell::getArea()const{
    return w * h;
}

double Cell::getScore()const{
    return score;
}

int Cell::getPinCount()const{
    return pinCount;
}

int Cell::getInputCount()const{
    return inputCount;
}

const std::string &Cell::getPinName(size_t pinIdx) const{
    if(pinIdx >= pinNames.size()) {
        throw std::out_of_range("Pin index out of range");
    }
    return pinNames[pinIdx];
}

const Coor &Cell::getPinCoor(const std::string &pinName)const{
    auto it = pinCoorMap.find(pinName);
    if (it == pinCoorMap.end()) {
        throw std::out_of_range("Pin name not found");
    }
    return it->second;
}

std::unordered_map<std::string, Coor> Cell::getPinCoorMap()const{
    return pinCoorMap;
}

double Cell::getQpinDelay() const{
    assert(type == Cell_Type::FF && "Not FF access");
    return QpinDelay;
}

double Cell::getGatePower() const{
    assert(type == Cell_Type::FF && "Not FF access");
    return GatePower;
}

std::ostream &operator<<(std::ostream &os, const Cell &cell){
    if(cell.type == Cell_Type::FF){
        os << "CellName: " << cell.cellName << std::endl;
        os << "Type: FF" << std::endl;
        os << "Number of bits: " << cell.bits << std::endl;
        os << "Width: " << cell.w << std::endl;
        os << "Height: " << cell.h << std::endl;
        os << "Pin Count: " << cell.pinCount << std::endl;
        os << "QpinDelay: " << cell.QpinDelay << std::endl;
        os << "GatePower: " << cell.GatePower << std::endl;
    }
    else if(cell.type == Cell_Type::Gate){
        os << "CellName: " << cell.cellName << std::endl;
        os << "Type: Gate" << std::endl;
        os << "Width: " << cell.w << std::endl;
        os << "Height: " << cell.h << std::endl;
        os << "Pin Count: " << cell.pinCount << std::endl;
    }
    else{
        os << "[UNSET Cell Detected!]" << std::endl;
    }

    for(const auto &pair : cell.pinCoorMap){
        os << "\t" << pair.first << " " << pair.second << std::endl;
    }
    return os;
}