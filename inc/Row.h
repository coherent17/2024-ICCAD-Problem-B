#ifndef _ROW_H_
#define _ROW_H_

#include "Subrow.h"
#include "Coor.h"
#include <vector>
#include <cmath>

// Row class used in legalizer code
// Will update the result of the struct::PlacementRow

class Row{
private:
    Coor startCoor;
    double siteHeight;
    double siteWidth;
    int numOfSites;
    std::vector<Subrow *> subrows;  // Use list

public:
    Row();
    ~Row();

    // Setters
    void setStartCoor(const Coor &startCoor);
    void setSiteHeight(double siteHeight);
    void setSiteWidth(double siteWidth);
    void setNumOfSite(int numOfSites);
    void addSubrows(Subrow *subrow);

    // Getters
    const Coor &getStartCoor()const;
    double getSiteHeight()const;
    double getSiteWidth()const;
    int getNumOfSite()const;
    const std::vector<Subrow *> &getSubrows()const;

    // If Node *gate overlap the current row, split current row
    void slicing(Node *gate);

    // Operator overloading for sorting
    bool operator<(const Row &rhs)const;

    friend std::ostream &operator<<(std::ostream &os, const Row &row);
};

#endif