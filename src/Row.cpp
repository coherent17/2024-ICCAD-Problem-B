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

// [TODO]: if the current row is overlap with the gate (fix comb cell), row will call this function
// Find the subrows in Row::subrows which is overlapped with gate, and split the effective subrows into multiple subrows
void Row::slicing(Node *gate) {
    // std::cout << *this << std::endl;
    // std::cout << "Cut with gate: " << *gate << std::endl;
    // Gate's coordinates and dimensions
    double gateStartX = gate->getGPCoor().x;
    double gateEndX = gateStartX + gate->getW();

    std::vector<Subrow *> newSubrows;
    for(auto subrow : subrows){
        double subrowStartX = subrow->getStartX();
        double subrowEndX = subrow->getEndX();

        // Check if subrow overlap with the gate
        if(subrowEndX > gateStartX && subrowStartX < gateEndX){

            // Part before the gate
            if(subrowStartX < gateStartX){
                Subrow *newSubrowBefore = new Subrow();
                newSubrowBefore->setStartX(subrowStartX);
                newSubrowBefore->setEndX(gateStartX);
                newSubrowBefore->setFreeWidth(newSubrowBefore->getEndX() - newSubrowBefore->getStartX());
                newSubrows.push_back(newSubrowBefore);
            }

            // Part after the gate
            if(subrowEndX > gateEndX){
                Subrow *newSubrowAfter = new Subrow();
                // make subrow startX always on site
                double alignedStartX = getStartCoor().x + std::ceil((int)(gateEndX - getStartCoor().x) / getSiteWidth()) * getSiteWidth();
                newSubrowAfter->setStartX(alignedStartX);
                newSubrowAfter->setEndX(subrowEndX);
                newSubrowAfter->setFreeWidth(newSubrowAfter->getEndX() - newSubrowAfter->getStartX());
                if(newSubrowAfter->getStartX() < newSubrowAfter->getEndX()){
                    newSubrows.push_back(newSubrowAfter);
                }
            }

        }
        else{
            // No overlap, keep the original subrow
            newSubrows.push_back(subrow);
        }

    }
    // Replace the old subrows with the new ones
    subrows = newSubrows;
    // std::cout << "After slicing:" << std::endl;
    // std::cout << *this << std::endl;
}

bool Row::canPlace(double startX, double endX){
    for(const auto &subrow : subrows){
        if(subrow->getStartX() <= startX && subrow->getEndX() >= endX){
            return true;
        }
    }
    return false;
}

bool Row::operator<(const Row &rhs)const{
    if(startCoor.y != rhs.startCoor.y){
        return startCoor.y < rhs.startCoor.y;
    }
    // Tie condition
    return startCoor.x < rhs.startCoor.x;
}

std::ostream &operator<<(std::ostream &os, const Row &row){
    os << "[ROW] ";
    os << "x: " << row.startCoor.x << ", ";
    os << "y: " << row.startCoor.y << ", ";
    os << "h: " << row.siteHeight << ", ";
    os << "w: " << row.siteWidth << ", ";
    os << "#sites: " << row.numOfSites << std::endl;
    for(size_t i = 0; i < row.subrows.size(); i++){
        os << "\t" << *(row.subrows[i]) << std::endl;
    }
    return os;
}