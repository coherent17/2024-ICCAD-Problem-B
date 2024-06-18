#include "Coor.h"

Coor::Coor(){
    x = 0;
    y = 0;
}
Coor::Coor(double x, double y){
    this->x = x;
    this->y = y;
}

Coor::~Coor(){
    ;
}

// setter
void Coor::setX(double x){
    this->x = x;
}

void Coor::setY(double y){
    this->y = y;
}

void Coor::setXY(double x, double y){
    this->x = x;
    this->y = y;
}

// getter
double Coor::getX()const{
    return x;
}

double Coor::getY()const{
    return y;
}

std::ostream &operator<<(std::ostream &out, const Coor &coor){
    out << "(" << coor.x << ", " << coor.y << ")";
    return out;
}

Coor Coor::operator+(const Coor& c) const{
    Coor temp;
    temp.x = this->x + c.x;
    temp.y = this->y + c.y;
    return temp;
}