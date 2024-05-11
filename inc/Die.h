#ifndef _DIE_H_
#define _DIE_H_

#include "Util.h"

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
    vector<PlacementRow> PlacementRows;

public:
    Die();
    ~Die();

    // setter
    void setDieOrigin(Coor &);
    void setDieBorder(Coor &);
    void setBinWidth(double);
    void setBinHeight(double);
    void setBinMaxUtil(double);
    void addPlacementRow(const PlacementRow &);

    // getter
    const Coor &getDieOrigin()const;
    const Coor &getDieBorder()const;
    double getBinWidth()const;
    double getBinHeight()const;
    double getBinMaxUtil()const;

    friend ostream &operator<<(ostream &, const Die &);
};

#endif