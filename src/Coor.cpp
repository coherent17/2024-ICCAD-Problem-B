#include "Coor.h"

Coor::Coor() : x(0), y(0){}

Coor::Coor(double x, double y){
    this->x = x;
    this->y = y;
}

Coor::~Coor(){
    ;
}

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

Coor Coor::operator+(const Coor& other) const{
    return Coor(x + other.x, y + other.y);
}

Coor Coor::operator-(const Coor& other) const{
    return Coor(x - other.x, y - other.y);
}