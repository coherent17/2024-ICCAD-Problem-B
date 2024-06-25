#include "FF.h"

FF::FF() : Instance(){
    ffIdx = UNSET_IDX;
    clusterIdx = UNSET_IDX;
    coor = {0, 0};
}

FF::~FF(){}

// Setter
void FF::setTimingSlack(const std::string &pinName, double slack){
    TimingSlack[pinName] = slack;
}

void FF::setFFIdx(int ffIdx){
    this->ffIdx = ffIdx;
}

void FF::setClusterIdx(int clusterIdx){
    this->clusterIdx = clusterIdx;
}

void FF::setNewCoor(const Coor &coor){
    this->newCoor = coor;
}

void FF::addNeighbor(int ffIdx, double euclidean_distance){
    NeighborFFs.push_back({ffIdx, euclidean_distance});
}

// Getter
double FF::getTimingSlack(const std::string &pinName)const{
    auto it = TimingSlack.find(pinName);
    if (it == TimingSlack.end()) {
        throw std::out_of_range("Pin name not found");
    }
    return it->second;
}

int FF::getFFIdx()const{
    return ffIdx;
}

int FF::getClusterIdx()const{
    return clusterIdx;
}

Coor FF::getNewCoor()const{
    return newCoor;
}

std::pair<int, double> FF::getNeighbor(int idx)const{
    return NeighborFFs[idx];
}

std::ostream &operator<<(std::ostream &os, const FF &ff){
    os << "Instance Name: " << ff.instanceName << std::endl;
    os << "Coor: " << ff.coor << std::endl;
    os << "CellName: " << ff.getCell()->getCellName() << std::endl;
    for(auto &pair : ff.TimingSlack)
        os << "Pin[" << pair.first << "] Slack: " << pair.second << std::endl;
    return os;
}
