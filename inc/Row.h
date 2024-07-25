#ifndef _ROW_H_
#define _ROW_H_

#include "Subrow.h"
#include "Coor.h"
#include "Node.h"
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
    double endX;
    std::vector<Subrow *> subrows;

public:
    Row();
    ~Row();

    // Setters
    void setStartCoor(const Coor &startCoor);
    void setSiteHeight(double siteHeight);
    void setSiteWidth(double siteWidth);
    void setNumOfSite(int numOfSites);
    void setEndX(double endX);
    void addSubrows(Subrow *subrow);

    // Getters
    const Coor &getStartCoor()const;
    double getSiteHeight()const;
    double getSiteWidth()const;
    int getNumOfSite()const;
    double getEndX()const;
    std::vector<Subrow *> &getSubrows();

    // Helper function

    /**
    @brief If Node *gate overlap with current row, split the current row.
    @param param can be either gate of ff
    @attention 
    3 types of condition must be consider
    a) Part before the gate
    b) Part under the gate
    c) Part after the gate
    @bug memory leak in this function, need to consider implement garbage collection
    */
    void slicing(Node *gate);

    /**
    @brief Check if the current row start from startX to endX have empty space with height h or not
    */
    bool canPlace(double startX, double endX, double &lowestH);

    /**
    @brief The cmp function for sort the placement row.
    @attention First sort by y in ascending order, and sort x in ascending order
    */
    bool operator<(const Row &rhs)const;

    friend std::ostream &operator<<(std::ostream &os, const Row &row);
};

#endif