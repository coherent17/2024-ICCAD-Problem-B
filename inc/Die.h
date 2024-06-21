#ifndef _DIE_H_
#define _DIE_H_

#include <iostream>
#include <vector>
#include "Coor.h"

struct PlacementRow{
    Coor startCoor;
    double siteWidth;
    double siteHeight;
    int NumOfSites;
};

class Die{
private:
    Coor DieOrigin;
    Coor DieBorder;
    double BinWidth;
    double BinHeight;
    double BinMaxUtil;
    std::vector<PlacementRow> PlacementRows;

public:
    Die();
    ~Die();

    // Setters
    void setDieOrigin(const Coor &);
    void setDieBorder(const Coor &);
    void setBinWidth(double);
    void setBinHeight(double);
    void setBinMaxUtil(double);
    void addPlacementRow(const PlacementRow &);

    // Getters
    const Coor &getDieOrigin()const;
    const Coor &getDieBorder()const;
    double getBinWidth()const;
    double getBinHeight()const;
    double getBinMaxUtil()const;
    const std::vector<PlacementRow>& getPlacementRows() const;

    // Stream Insertion
    friend std::ostream &operator<<(std::ostream &, const Die &);
};

#endif