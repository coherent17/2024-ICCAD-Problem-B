#ifndef _FF_H_
#define _FF_H_

#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include "Instance.h"
#include "Manager.h"
#include "Util.h"

class Manager;

class FF : public Instance{
private:
    std::unordered_map<std::string, double> TimingSlack;
    
    // ######################################### used in cluster ########################################################
    int ffIdx;
    int clusterIdx;
    Coor newCoor;
    double bandwidth;   // used in gaussian kernel function
    bool isShifting;
    // pair<other ffId, euclidean distance>, store the neighbor ff with their Id and the distance to this FF
    std::vector<std::pair<int, double>> NeighborFFs;
    // ######################################### used in cluster ########################################################

    // // for find optimal location
    // Cell* prev_cell;
    // FF* prev_ff;
    // int idx;

    Coor originalD, originalQ; // initial location for FF list, only can be set in mgr.Debank
    double originalQpinDelay;
public:
    FF();
    ~FF();

    // Setters
    void setTimingSlack(const std::string &pinName, double slack);
    void setFFIdx(int ffIdx);
    void setClusterIdx(int clusterIdx);
    void setNewCoor(const Coor &coor);
    void setBandwidth();
    void addNeighbor(int ffIdx, double euclidean_distance);
    void setIsShifting(bool shift);
    void setOriginalCoor(const Coor& coorD, const Coor& coorQ);
    void setOriginalQpinDelay(double);
    //void setIdx(int i){this->idx = i;}
    
    // Getter
    double getTimingSlack(const std::string &pinName)const;
    int getFFIdx()const;
    bool getIsCluster()const;
    int getClusterIdx()const;
    Coor getNewCoor()const;
    double getBandwidth()const;
    std::pair<int, double> getNeighbor(int idx)const;
    int getNeighborSize()const;
    bool getIsShifting()const;
    Coor getOriginalD()const;
    Coor getOriginalQ()const;
    double getOriginalQpinDelay()const;
    //int getIdx(){return this->idx;}
    
    // ######################################### used in cluster ########################################################
    void sortNeighbors();
    double shift(const Manager &mgr);     // shift the ff and return the euclidean distance from origin coordinate
    // ######################################### used in cluster ########################################################

    friend std::ostream &operator<<(std::ostream &os, const FF &ff);
};


#endif
