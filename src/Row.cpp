#include "Row.h"

Row::Row() :startCoor(Coor(0, 0)){
    siteHeight = 0;
    siteWidth = 0;
    numOfSites = 0;
    endX = 0;
}

Row::~Row(){
    for(auto &subrow : subrows){
        delete subrow;
    }
    subrows.clear();
}

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

void Row::setEndX(double endX){
    this->endX = endX;
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

double Row::getEndX()const{
    return endX;
}

std::vector<Subrow *> &Row::getSubrows(){
    return subrows;
}

void Row::slicing(Node *gate) {
    assert(subrows.size() != 0 && "No more subrow can be used in this row!");
    double gateStartX = gate->getLGCoor().x;
    double gateEndX = gateStartX + gate->getW();
    double gateStartY = gate->getLGCoor().y;
    double rowStartY = getStartCoor().y;

    // check if gate is above or below the current row
    if(gateStartY >= rowStartY + siteHeight || gateStartY + gate->getH() <= rowStartY){
        return;
    }

    std::vector<Subrow *> newSubrows;
    for(auto subrow : subrows){
        double subrowStartX = subrow->getStartX();
        double subrowEndX = subrow->getEndX();
        double subrowHeight = subrow->getHeight();

        // Check if subrow overlap with the gate
        if(subrowEndX > gateStartX && subrowStartX < gateEndX && gateStartY < rowStartY + subrowHeight){

            //a) Part before the gate
            if(subrowStartX < gateStartX){
                Subrow *newSubrowBefore = new Subrow();
                newSubrowBefore->setStartX(subrowStartX);
                newSubrowBefore->setEndX(gateStartX);
                newSubrowBefore->setFreeWidth(newSubrowBefore->getEndX() - newSubrowBefore->getStartX());
                newSubrowBefore->setHeight(subrowHeight);
                newSubrows.push_back(newSubrowBefore);
            }
            
            //b) Part under the gate
            if(gateStartY > rowStartY){
                double newSubrowStartX = (gateStartX <= subrowStartX) ? subrowStartX : gateStartX;
                double newSubrowEndX = (gateEndX >= subrowEndX) ? subrowEndX : gateEndX;                
                Subrow *newSubrow = new Subrow();
                newSubrow->setStartX(newSubrowStartX);
                newSubrow->setEndX(newSubrowEndX);
                newSubrow->setFreeWidth(newSubrow->getEndX() - newSubrow->getStartX());
                newSubrow->setHeight(gateStartY - rowStartY);
                newSubrows.push_back(newSubrow);
            }

            //c) Part after the gate
            if(subrowEndX > gateEndX){
                Subrow *newSubrowAfter = new Subrow();
                newSubrowAfter->setStartX(gateEndX);
                newSubrowAfter->setEndX(subrowEndX);
                newSubrowAfter->setFreeWidth(newSubrowAfter->getEndX() - newSubrowAfter->getStartX());
                newSubrowAfter->setHeight(subrowHeight);
                newSubrows.push_back(newSubrowAfter);
            }
        }
        else{
            // No overlap, keep the original subrow
            newSubrows.push_back(subrow);
        }
    }
    // Replace the old subrows with the new ones
    subrows = newSubrows;
}

bool Row::canPlace(double startX, double endX, double h){
    double traverseX = startX;
    for(const auto &subrow : subrows){
        // early break condition
        if(subrow->getStartX() > startX) break;

        // check if can use subrow to cover startX ~ endX with enough height h
        if(subrow->getStartX() <= traverseX && subrow->getEndX() >= traverseX && subrow->getHeight() >= h){
            traverseX = subrow->getEndX();
        }
        if(traverseX >= endX){
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