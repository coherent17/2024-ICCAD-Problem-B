#ifndef _COOR_H_
#define _COOR_H_

#include <iostream>
using namespace std;

class Coor{
private:
    double x;
    double y;

public:
    Coor();
    Coor(double, double);
    ~Coor();

    // setter
    void setX(double);
    void setY(double);
    void setXY(double, double);

    // getter
    double getX()const;
    double getY()const;
    friend ostream &operator<<(ostream &, const Coor &);
};

#endif