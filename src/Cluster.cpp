#include "Cluster.h"

Cluster::Cluster(){
    this->clusterIdx = UNSET_IDX;
    this->coor = {0, 0};
}

Cluster::~Cluster(){}

// Setters
void Cluster::setClusterIdx(int clusterIdx){
    this->clusterIdx = clusterIdx;
}

void Cluster::setCoor(const Coor &coor){
    this->coor = coor;
}

void Cluster::addFF(int ffIdx){
    FFs.push_back(ffIdx);
}

// Getters
int Cluster::getClusterIdx()const{
    return clusterIdx;
}

Coor Cluster::getCoor()const{
    return coor;
}

int Cluster::getFF(int idx)const{
    assert(idx >= 0 && idx < (int)FFs.size());
    return FFs[idx];
}