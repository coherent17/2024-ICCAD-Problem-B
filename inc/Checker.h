#ifndef _CHECKER_H_
#define _CHECKER_H_

#ifdef ENABLE_DEBUG_CHECKER
#define DEBUG_CHECKER(message) std::cout << "[CHECKER] " << message << std::endl
#else
#define DEBUG_CHECKER(message)
#endif

#include "Manager.h"

struct Rect{
    double startX;
    double startY;
    double endX;
    double endY;
    string instName;
};

class Checker{
private:
    Manager& mgr;
    vector<Rect> FFs;
    vector<Rect> Gates;
    // [TODO] Construct the placement info from Manager::die

public:
    explicit Checker(Manager& mgr);
    ~Checker();
    void run();

private:
    void initialChecker();
    void checkOnSite();
    void checkOverlap();
    void checkDieBoundary();
    static bool overlap(Rect rect1, Rect rect2);
    static void sortRects(std::vector<Rect> &rect);
};

#endif