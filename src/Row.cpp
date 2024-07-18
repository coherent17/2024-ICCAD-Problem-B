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

std::vector<Subrow *> &Row::getSubrows(){
    return subrows;
}

// [TODO]: if the current row is overlap with the gate (fix comb cell), row will call this function
// Find the subrows in Row::subrows which is overlapped with gate, and split the effective subrows into multiple subrows
void Row::slicing(Node *gate) {
    if(subrows.size() == 0){
        std::cout << "I am here" << std::endl;
    }
    // std::cout << *this << std::endl;
    // std::cout << "Cut with gate: " << *gate << std::endl;
    // Gate's coordinates and dimensions
    double gateStartX = gate->getLGCoor().x;
    double gateEndX = gateStartX + gate->getW();
    double gateStartY = gate->getLGCoor().y;
    double rowStartY = getStartCoor().y;

    if(gateStartY >= rowStartY+ siteHeight || gateStartY + gate->getH() <= rowStartY){
        return;
    }// todo revise siteHeight

    std::vector<Subrow *> newSubrows;
    for(auto subrow : subrows){
        double subrowStartX = subrow->getStartX();
        double subrowEndX = subrow->getEndX();
        double subrowHeight = subrow->getHeight();

        // Check if subrow overlap with the gate
        if(subrowEndX > gateStartX && subrowStartX < gateEndX){

            // Part before the gate
            if(subrowStartX < gateStartX){
                Subrow *newSubrowBefore = new Subrow();
                newSubrowBefore->setStartX(subrowStartX);
                newSubrowBefore->setEndX(gateStartX);
                newSubrowBefore->setFreeWidth(newSubrowBefore->getEndX() - newSubrowBefore->getStartX());
                newSubrowBefore->setHeight(subrowHeight);
                newSubrows.push_back(newSubrowBefore);
                // std::cout << "StartX: " << newSubrowBefore->getStartX() << std::endl;
                // std::cout << "EndX: " << newSubrowBefore->getEndX() << std::endl;
                // std::cout << "H: " << newSubrowBefore->getHeight() << std::endl;

            }
            // Part under gate 
            if(gateStartY > rowStartY){
                // std::cout << "Under Gate\n";
                double newSubrowStartX;
                double newSubrowEndX;
                if(gateStartX <= subrowStartX){
                    newSubrowStartX = subrowStartX;
                }else{
                    // double alignedStartX = subrowStartX + std::ceil((int)(gateStartX - subrowStartX) / getSiteWidth()) * getSiteWidth();
                    newSubrowStartX = gateStartX;
                } 
                if(gateEndX >= subrowEndX){
                    newSubrowEndX = subrowEndX;
                }else{
                    newSubrowEndX = gateEndX;
                }
                
                Subrow *newSubrow = new Subrow();
                newSubrow->setStartX(newSubrowStartX);
                newSubrow->setEndX(newSubrowEndX);
                newSubrow->setFreeWidth(newSubrow->getEndX() - newSubrow->getStartX());
                newSubrow->setHeight(gateStartY - rowStartY);
                // std::cout << "StartX: " << newSubrow->getStartX() << std::endl;
                // std::cout << "EndX: " << newSubrow->getEndX() << std::endl;
                // std::cout << "H: " << newSubrow->getHeight() << std::endl;
                newSubrows.push_back(newSubrow);
            }

            // Part after the gate
            if(subrowEndX > gateEndX){
                Subrow *newSubrowAfter = new Subrow();
                // make subrow startX always on site
                // double alignedStartX = getStartCoor().x + std::ceil((int)(gateEndX - getStartCoor().x) / getSiteWidth()) * getSiteWidth();
                newSubrowAfter->setStartX(gateEndX);
                newSubrowAfter->setEndX(subrowEndX);
                newSubrowAfter->setFreeWidth(newSubrowAfter->getEndX() - newSubrowAfter->getStartX());
                newSubrowAfter->setHeight(subrowHeight);
                if(newSubrowAfter->getStartX() <= newSubrowAfter->getEndX()){
                    newSubrows.push_back(newSubrowAfter);
                    // std::cout << "StartX: " << newSubrowAfter->getStartX() << std::endl;
                    // std::cout << "EndX: " << newSubrowAfter->getEndX() << std::endl;
                    // std::cout << "H: " << newSubrowAfter->getHeight() << std::endl;
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

bool Row::canPlace(double startX, double endX, double h){
    double traverseX = startX;
    for(const auto &subrow : subrows){
        if(subrow->getStartX() <= traverseX && subrow->getEndX() >= traverseX && subrow->getHeight() >= h){
            traverseX = subrow->getEndX();
            // std::cout << "X:" << traverseX << std::endl;
            // std::cout << "Y:" << subrow->getHeight() << std::endl;
        }
        if(traverseX >= endX){
            return true;
        }
        // if(subrow->getStartX() <= startX && subrow->getEndX() >= endX){
        //     return true;
        // }
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