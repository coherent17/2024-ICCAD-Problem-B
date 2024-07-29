#include "Subrow.h"

Subrow::Subrow(){
    startX = 0;
    endX = 0;
    freeWidth = 0;
    height = 0;
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

void Subrow::setHeight(double height){
    this->height = height;
}

void Subrow::addRejectCell(Cell *cell){
    reject_cells.insert(cell);
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

double Subrow::getHeight()const{
    return height;
}

bool Subrow::hasCell(Cell *cell){
    if(reject_cells.count(cell)){
        return true;
    }
    return false;
}

std::ostream &operator<<(std::ostream &os, const Subrow &subrow){
    os << "[Subrow] ";
    os << "(startX: " << subrow.startX << ", ";
    os << "endX: " << subrow.endX << ")";
    os << " height: " << subrow.height;
    return os;
}