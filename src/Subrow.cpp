#include "Subrow.h"

Subrow::Subrow(){
    startX = 0;
    endX = 0;
    freeWidth = 0;
    lastCluster = nullptr;
}

Subrow::~Subrow(){
    lastCluster = nullptr;
}

// Setters
void Subrow::setStartX(double startX){
    this->startX = startX;
}

void Subrow::setEndX(double endX){
    this->endX = endX;
}

void Subrow::setFreeWidth(double freeWidth){
    this->freeWidth = freeWidth;
}

void Subrow::setLastCluster(SubrowCluster *lastCluster){
    this->lastCluster = lastCluster;
}

// Getters
double Subrow::getStartX()const{
    return startX;
}

double Subrow::getEndX()const{
    return endX;
}

double Subrow::getFreeWidth()const{
    return freeWidth;
}

SubrowCluster *Subrow::getLastCluster()const{
    return lastCluster;
}

std::ostream &operator<<(std::ostream &os, const Subrow &subrow){
    os << "[Subrow] ";
    os << "(startX: " << subrow.startX << ", ";
    os << "endX: " << subrow.endX << ")";
    return os;
}