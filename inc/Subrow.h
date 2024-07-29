#ifndef _SUBROW_H_
#define _SUBROW_H_

// Subrow class used in legalizer code
#include <iostream>
#include <set>
#include "Cell.h"

class Subrow{
private:
    double startX;
    double endX;
    double freeWidth;
    double height;
    std::set<Cell *> reject_cells;

public:
    Subrow();
    ~Subrow();

    // Setters
    void setStartX(double startX);
    void setEndX(double endX);
    void setFreeWidth(double freeWidth);
    void setHeight(double height);
    void addRejectCell(Cell *cell);

    // Getters
    double getStartX()const;
    double getEndX()const;
    double getFreeWidth()const;
    double getHeight()const;
    bool hasCell(Cell *cell);

    friend std::ostream &operator<<(std::ostream &os, const Subrow &subrow);
};

#endif