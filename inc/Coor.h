#ifndef _COOR_H_
#define _COOR_H_

#include <iostream>

class Coor{
public:
    double x;
    double y;

public:
    Coor();
    Coor(double x, double y);
    virtual ~Coor();

    // Setters
    void setX(double x);
    void setY(double y);
    void setXY(double x, double y);

    // Getters
    double getX()const;
    double getY()const;

    // Stream Insertion
    friend std::ostream &operator<<(std::ostream &os, const Coor &coor);

    // Operator Overloading
    Coor operator+(const Coor& other) const;
    Coor operator-(const Coor& other) const;
};

#endif