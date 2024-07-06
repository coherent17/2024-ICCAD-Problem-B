#include "Die.h"

Die::Die() : 
    DieOrigin(0, 0), 
    DieBorder(0, 0), 
    BinWidth(0),
    BinHeight(0),
    BinMaxUtil(0) 
    {}

Die::~Die(){}

// Setters
void Die::setDieOrigin(const Coor &coor){
    this->DieOrigin = coor;
}

void Die::setDieBorder(const Coor &coor){
    this->DieBorder = coor;
}

void Die::setBinWidth(double BinWidth){
    this->BinWidth = BinWidth;
}

void Die::setBinHeight(double BinHeight){
    this->BinHeight = BinHeight;
}

void Die::setBinMaxUtil(double BinMaxUtil){
    this->BinMaxUtil = BinMaxUtil;
}

void Die::addPlacementRow(const PlacementRow &placementrow){
    this->PlacementRows.push_back(placementrow);
}

const Coor &Die::getDieOrigin()const{
    return DieOrigin;
}

const Coor &Die::getDieBorder()const{
    return DieBorder;
}

double Die::getBinWidth()const{
    return BinWidth;
}

double Die::getBinHeight()const{
    return BinHeight;
}

double Die::getBinMaxUtil()const{
    return BinMaxUtil;
}

const std::vector<PlacementRow>& Die::getPlacementRows() const{
    return  PlacementRows;
}

std::ostream &operator<<(std::ostream &os, const Die &die){
    os << "DieOrigin: " << die.DieOrigin << std::endl;
    os << "DieBorder: " << die.DieBorder << std::endl;
    os << "BinWidth: " << die.BinWidth << std::endl;
    os << "BinHeight: " << die.BinHeight << std::endl;
    os << "BinMaxUtil: " << die.BinMaxUtil << std::endl;

    os << "Placement Rows:\n";
    for (const auto& row : die.PlacementRows) {
        os << "  Start Coor: " << row.startCoor
           << ", Site Width: " << row.siteWidth
           << ", Site Height: " << row.siteHeight
           << ", Number of Sites: " << row.NumOfSites << "\n";
    }
    return os;
}