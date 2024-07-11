#include "Cluster.h"

Cluster::Cluster(){
    // this->clusterIdx = UNSET_IDX;
    this->coor = {0, 0};
    cell = nullptr;
}

Cluster::~Cluster(){}

// Setters
// void Cluster::setClusterIdx(int clusterIdx){
//     this->clusterIdx = clusterIdx;
// }

void Cluster::setCoor(const Coor &coor){
    this->coor = coor;
}

void Cluster::addFF(FF* FF){
    FFs.push_back(FF);
}


void Cluster::setCell(Cell* cell){
    this->cell = cell;
}

// Getters
// int Cluster::getClusterIdx()const{
//     return clusterIdx;
// }

Coor Cluster::getCoor()const{
    return coor;
}

std::vector<FF *> Cluster::getFFs()const{
    return FFs;
}


Cell* Cluster::getCell()const{
    return cell;
}