#ifndef _SUBROW_H_
#define _SUBROW_H_

// Subrow class used in legalizer code
#include <iostream>

class Subrow{
private:
    double startX;
    double endX;
    double freeWidth;

public:
    Subrow();
    ~Subrow();

    // Setters
    void setStartX(double startX);
    void setEndX(double endX);
    void setFreeWidth(double freeWidth);

    // Getters
    double getStartX()const;
    double getEndX()const;
    double getFreeWidth()const;

    friend std::ostream &operator<<(std::ostream &os, const Subrow &subrow);
};

#endif