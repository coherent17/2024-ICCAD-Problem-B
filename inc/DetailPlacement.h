#ifndef _DETAIL_PLACEMENT_H_
#define _DETAIL_PLACEMENT_H_

#include "Node.h"
#include "Subrow.h"
#include "Row.h"
#include "Manager.h"
#include <iostream>
#include <vector>

#define DEBUG_DP(message) std::cout << "[DetailPlacement] " << message << std::endl

class DetailPlacement{
private:
    Manager &mgr;
    std::vector<Node *> ffs;
    std::vector<Node *> gates;
    std::vector<Row *> rows;

public:
    explicit DetailPlacement(Manager &mgr);
    ~DetailPlacement();

    void run();

private:
    void GlobalSwap();
    void VerticalSwap();
    void LoaclReordering();
};


#endif