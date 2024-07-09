#include "SubrowCluster.h"

SubrowCluster::SubrowCluster(){
    x = 0;
    q = 0;
    w = 0;
    e = 0;
    prev = nullptr;
}

SubrowCluster::~SubrowCluster(){
    cells.clear();
    prev = nullptr;
}

// Setters
void SubrowCluster::setX(double x){
    this->x = x;
}

void SubrowCluster::setQ(double q){
    this->q = q;
}

void SubrowCluster::setW(double w){
    this->w = w;
}

void SubrowCluster::setE(double e){
    this->e = e;
}

void SubrowCluster::setPrev(SubrowCluster *prev){
    this->prev = prev;
}

void SubrowCluster::addCells(Node *cell){
    cells.push_back(cell);
    e += cell->getWeight();
    q += cell->getWeight() * (cell->getGPCoor().x - w);
    w += cell->getW();
}

// Getters
double SubrowCluster::getX()const{
    return x;
}

double SubrowCluster::getQ()const{
    return q;
}

double SubrowCluster::getW()const{
    return w;
}

double SubrowCluster::getE()const{
    return e;
}

const std::vector<Node *> &SubrowCluster::getCells()const{
    return cells;
}

std::ostream &operator<<(std::ostream &os, const SubrowCluster &subrowcluster){
    os << "[Cluster] ";
    os << "X: " << subrowcluster.x << ", ";
    os << "Q: " << subrowcluster.q << ", ";
    os << "W: " << subrowcluster.w << ", ";
    os << "E: " << subrowcluster.e << std::endl;
    return os;
}