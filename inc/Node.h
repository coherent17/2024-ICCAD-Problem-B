#ifndef _NODE_H_
#define _NODE_H_

#include "Coor.h"
#include <string.h>

// Node class used in legalizer code, 
// represent about the ff and the comb cell(obstacle)

class Node{
private:
    std::string name;
    Coor gpCoor;   // global placement coordinate
    Coor lgzCoor;  // legalized coordinatge
    double w;
    double h;
    double weight;

public:
    Node();
    ~Node();
    
    // Setters
    void setName(const std::string &name);
    void setGPCoor(const Coor &gpCoor);
    void setLGZCoor(const Coor &lgzCoor);
    void setW(double w);
    void setH(double h);
    void setWeight(double weight);

    // Getters
    const std::string &getName()const;
    const Coor &getGPCoor()const;
    const Coor &getLHZCoor()const;
    double getW()const;
    double getH()const;
    double getWeight()const;

    // Stream Insertion
    friend std::ostream &operator<<(std::ostream &os, const Node &node);
};

#endif