#include "Row.h"

Row::Row(){
    startCoor = {0, 0};
    siteHeight = 0;
    siteWidth = 0;
    numOfSites = 0;
}

Row::~Row(){}

// Setters
void Row::setStartCoor(const Coor &startCoor){
    this->startCoor = startCoor;
}

void Row::setSiteHeight(double siteHeight){
    this->siteHeight = siteHeight;
}

void Row::setSiteWidth(double siteWidth){
    this->siteWidth = siteWidth;
}

void Row::setNumOfSite(int numOfSites){
    this->numOfSites = numOfSites;
}

void Row::addSubrows(Subrow *subrow){
    subrows.push_back(subrow);
}

// Getters
const Coor &Row::getStartCoor()const{
    return startCoor;
}

double Row::getSiteHeight()const{
    return siteHeight;
}

double Row::getSiteWidth()const{
    return siteWidth;
}

int Row::getNumOfSite()const{
    return numOfSites;
}

const std::vector<Subrow *> &Row::getSubrows()const{
    return subrows;
}