#ifndef _SUBROW_CLUSTER_H_
#define _SUBROW_CLUSTER_H_

#include "Node.h"
#include <vector>

// SubrowCluster class used in legalizer code, 

class SubrowCluster{
private:
    double x; // Optimal position (lower left corner), x_c in paper
    double q; // q_c / e_c = x_c
    double w; // Total width, w_c in paper
    double e; // Total weight, e_c in paper
    std::vector<Node *> cells;
    SubrowCluster *prev;

public:
    SubrowCluster();
    ~SubrowCluster();

    // Setters
    void setX(double x);
    void setQ(double q);
    void setW(double w);
    void setE(double e);
    void setPrev(SubrowCluster *prev);
    void addCells(Node *cell);

    // Getters
    double getX()const;
    double getQ()const;
    double getW()const;
    double getE()const;
    const std::vector<Node *> &getCells()const;

    friend std::ostream &operator<<(std::ostream &os, const SubrowCluster &subrowcluster);
};

#endif