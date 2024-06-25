#ifndef _CLUSTER_H_
#define _CLUSTER_H_

#include <vector>
#include <cassert>
#include "Coor.h"
#include "Util.h"

class Cluster{
private:
    int clusterIdx;
    Coor coor;
    std::vector<int> FFs;

public:
    Cluster();
    ~Cluster();

    // Setters
    void setClusterIdx(int clusterIdx);
    void setCoor(const Coor &coor);
    void addFF(int ffIdx);

    // Getters
    int getClusterIdx()const;
    Coor getCoor()const;
    int getFF(int idx)const;

};

#endif