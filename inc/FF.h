#ifndef _FF_H_
#define _FF_H_

#include <iostream>
#include <string>
#include <unordered_map>
#include "Instance.h"
#include "Util.h"

class FF : public Instance{
private:
    std::unordered_map<std::string, double> TimingSlack;

    // ######################################### used in cluster ########################################################
    int ffIdx;
    int clusterIdx;
    Coor newCoor;

    // pair<other ffId, euclidean distance>, store the neighbor ff with their Id and the distance to this FF
    std::vector<std::pair<int, double>> NeighborFFs;
    // ######################################### used in cluster ########################################################

    // // for find optimal location
    // Cell* prev_cell;
    // FF* prev_ff;
    // int idx;

public:
    FF();
    ~FF();

    // Setters
    void setTimingSlack(const std::string &pinName, double slack);
    void setFFIdx(int ffIdx);
    void setClusterIdx(int clusterIdx);
    void setNewCoor(const Coor &coor);
    void addNeighbor(int ffIdx, double euclidean_distance);
    //void setIdx(int i){this->idx = i;}
    
    // Getter
    double getTimingSlack(const std::string &pinName)const;
    int getFFIdx()const;
    int getClusterIdx()const;
    Coor getNewCoor()const;
    std::pair<int, double> getNeighbor(int idx)const;
    //int getIdx(){return this->idx;}
    

    friend std::ostream &operator<<(std::ostream &os, const FF &ff);
};


#endif
