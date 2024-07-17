#include "Subrow.h"

Subrow::Subrow(){
    startX = 0;
    endX = 0;
    freeWidth = 0;
}

Subrow::~Subrow(){}

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

std::ostream &operator<<(std::ostream &os, const Subrow &subrow){
    os << "[Subrow] ";
    os << "(startX: " << subrow.startX << ", ";
    os << "endX: " << subrow.endX << ")";
    return os;
}