#ifndef _BANKING_H_
#define _BANKING_H_

#include <vector>
#include "Cluster.h"
#include "Manager.h"
#include "Util.h"
#include "FF.h"

class Manager;
class Cluster;
class FF;

class Banking{
private:
    Manager& mgr;
public:
    explicit Banking(Manager& mgr);
    ~Banking();

    void run();
    // void lib_scoring();
    Coor getMedian(FF* nowFF, Cluster& c, std::vector<int>& resultFFs, std::vector<int>& toRemoveFFs);

};

#endif