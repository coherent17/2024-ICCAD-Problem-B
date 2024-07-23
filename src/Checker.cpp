#include "Checker.h"

Checker::Checker(Manager &mgr) :mgr(mgr){}

Checker::~Checker(){}

bool Checker::run(){
    DEBUG_CHECKER("Start Checker");
    return (checkOnSite() && checkOverlap() && checkDieBoundary());
}

bool Checker::checkOnSite(){
    DEBUG_CHECKER("Check On Site");
    return true;
}

bool Checker::checkOverlap(){
    DEBUG_CHECKER("Check Overlap");
    return true;
}

bool Checker::checkDieBoundary(){
    DEBUG_CHECKER("Check Die Boundary");
    return true;
}