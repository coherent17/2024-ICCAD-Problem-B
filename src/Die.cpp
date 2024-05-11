#include "Die.h"

Die::Die(){
    this->BinWidth = 0;
    this->BinHeight = 0;
    this->BinMaxUtil = 0;
}

Die::~Die(){
    ;
}

void Die::setDieOrigin(Coor &coor){
    this->DieOrigin = coor;
}

void Die::setDieBorder(Coor &coor){
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
    this->PlacementRows.emplace_back(placementrow);
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

ostream &operator<<(ostream &out, const Die &die){
    out << "DieOrigin: " << die.DieOrigin << endl;
    out << "DieBorder: " << die.DieBorder << endl;
    out << "BinWidth: " << die.BinWidth << endl;
    out << "BinHeight: " << die.BinHeight << endl;
    out << "BinMaxUtil: " << die.BinMaxUtil << endl;
    return out;
}