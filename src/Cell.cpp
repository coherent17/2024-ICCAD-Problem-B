#include "Cell.h"

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

Coor &Cell::getPinCoor(string pinName)const{
    return pinCoorMap[pinName];
}