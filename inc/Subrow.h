#ifndef _SUBROW_H_
#define _SUBROW_H_

#include "SubrowCluster.h"

// Subrow class used in legalizer code

class Subrow{
private:
    double startX;
    double endX;
    double freeWidth;
    SubrowCluster *lastCluster;

public:
    Subrow();
    ~Subrow();

    // Setters
    void setStartX(double startX);
    void setEndX(double endX);
    void setFreeWidth(double freeWidth);
    void setLastCluster(SubrowCluster *lastCluster);

    // Getters
    double getStartX()const;
    double getEndX()const;
    double getFreeWidth()const;
    SubrowCluster *getLastCluster()const;

    friend std::ostream &operator<<(std::ostream &os, const Subrow &subrow);
};

#endif