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
// Memory leak in this function...
void Row::slicing(Node *gate) {
    // Gate's coordinates and dimensions
    double gateStartX = gate->getGPCoor().x;
    double gateEndX = gateStartX + gate->getW();

    std::vector<Subrow *> newSubrows;
    const double tolerance = 1e-6;  // Small tolerance for numerical precision

    for (Subrow *subrow : subrows) {
        double subrowStartX = subrow->getStartX();
        double subrowEndX = subrow->getEndX();

        // Check if the subrow overlaps with the gate
        if (subrowEndX > gateStartX + tolerance && subrowStartX < gateEndX - tolerance) {
            // Create new subrows based on the overlap

            // Case 1: Part before the gate
            if (subrowStartX < gateStartX) {
                double newSubrowEndX = std::min(gateStartX, subrowEndX);
                if (newSubrowEndX > subrowStartX + tolerance) {
                    // Align the new subrow to the nearest valid start and end positions
                    double alignedStartX = getStartCoor().x + ((int)((subrowStartX - getStartCoor().x) / getSiteWidth())) * getSiteWidth();
                    double alignedEndX = getStartCoor().x + ((int)((newSubrowEndX - getStartCoor().x + getSiteWidth() - 1) / getSiteWidth())) * getSiteWidth();

                    Subrow *newSubrowBefore = new Subrow();
                    newSubrowBefore->setStartX(alignedStartX);
                    newSubrowBefore->setEndX(alignedEndX);
                    newSubrowBefore->setFreeWidth(newSubrowBefore->getEndX() - newSubrowBefore->getStartX());
                    newSubrowBefore->setLastCluster(nullptr); // Adjust as needed
                    newSubrows.push_back(newSubrowBefore);
                }
            }

            // Case 2: Part after the gate
            if (subrowEndX > gateEndX + tolerance) {
                double newSubrowStartX = std::max(gateEndX, subrowStartX);
                if (newSubrowStartX < subrowEndX - tolerance) {
                    // Align the new subrow to the nearest valid start and end positions
                    double alignedStartX = getStartCoor().x + ((int)((newSubrowStartX - getStartCoor().x) / getSiteWidth())) * getSiteWidth();
                    double alignedEndX = getStartCoor().x + ((int)((subrowEndX - getStartCoor().x + getSiteWidth() - 1) / getSiteWidth())) * getSiteWidth();

                    Subrow *newSubrowAfter = new Subrow();
                    newSubrowAfter->setStartX(alignedStartX);
                    newSubrowAfter->setEndX(alignedEndX);
                    newSubrowAfter->setFreeWidth(newSubrowAfter->getEndX() - newSubrowAfter->getStartX());
                    newSubrowAfter->setLastCluster(nullptr); // Adjust as needed
                    newSubrows.push_back(newSubrowAfter);
                }
            }
        } else {
            // No overlap, keep the original subrow
            newSubrows.push_back(subrow);
        }
    }

    // Replace the old subrows with the new ones
    subrows = newSubrows;
}
std::ostream &operator<<(std::ostream &os, const Row &row){
    os << "[ROW] ";
    os << "x: " << row.startCoor.x << ", ";
    os << "y: " << row.startCoor.y << ", ";
    os << "h: " << row.siteHeight << ", ";
    os << "w: " << row.siteHeight << ", ";
    os << "#sites: " << row.numOfSites << std::endl;
    for(size_t i = 0; i < row.subrows.size(); i++){
        os << "\t" << *(row.subrows[i]) << std::endl;
    }
    return os;
}