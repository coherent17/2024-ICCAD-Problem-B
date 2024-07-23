#ifndef _CHECKER_H_
#define _CHECKER_H_

#define DEBUG_CHECKER(message) std::cout << "[CHECKER] " << message << std::endl

#include "Manager.h"

class Checker{
private:
    Manager& mgr;
    // [TODO] Construct the placement info from Manager::die

public:
    explicit Checker(Manager& mgr);
    ~Checker();
    bool run();

private:
    bool checkOnSite();
    bool checkOverlap();
    bool checkDieBoundary();
};

#endif