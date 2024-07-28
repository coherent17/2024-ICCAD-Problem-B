#include "Node.h"
#include <cfloat>

Node::Node() :name(""), gpCoor(0, 0), lgCoor(DBL_MAX, DBL_MAX){
    cell = nullptr;
    w = 0;
    h = 0;
    isPlace = false;
    TNS = 0;
}

Node::~Node(){}

// Setters
void Node::setName(const std::string &name){
    this->name = name;
}

void Node::setGPCoor(const Coor &gpCoor){
    this->gpCoor = gpCoor;
}

void Node::setLGCoor(const Coor &lgCoor){
    this->lgCoor = lgCoor;
}

void Node::setCell(Cell *cell){
    this->cell = cell;
}

void Node::setW(double w){
    this->w = w;
}

void Node::setH(double h){
    this->h = h;
}

void Node::setIsPlace(bool isPlace){
    this->isPlace = isPlace;
}

void Node::setTNS(double TNS){
    this->TNS = TNS;
}

void Node::setClosestRowIdx(size_t closest_rowidx){
    this->closest_rowidx = closest_rowidx;
}

void Node::setPlaceRowIdx(size_t place_rowidx){
    this->place_rowidx = place_rowidx;
}

// Getters
const std::string &Node::getName()const{
    return name;
}

const Coor &Node::getGPCoor()const{
    return gpCoor;
}

const Coor &Node::getLGCoor()const{
    return lgCoor;
}

Cell *Node::getCell()const{
    return cell;
}

double Node::getW()const{
    return w;
}

double Node::getH()const{
    return h;
}

bool Node::getIsPlace()const{
    return isPlace;
}

double Node::getDisplacement()const{
    return std::sqrt(std::pow(gpCoor.x - lgCoor.x, 2) + std::pow(gpCoor.y - lgCoor.y, 2));
}

double Node::getDisplacement(const Coor &otherCoor)const{
    return std::sqrt(std::pow(gpCoor.x - otherCoor.x, 2) + std::pow(gpCoor.y - otherCoor.y, 2));
}

double Node::getTNS()const{
    return TNS;
}

size_t Node::getClosestRowIdx()const{
    return closest_rowidx;
}

size_t Node::getPlaceRowIdx()const{
    return place_rowidx;
}

std::ostream &operator<<(std::ostream &os, const Node &node){
    os << "Name: " << node.name << std::endl;
    os << "GPCoor: " << node.gpCoor << std::endl;
    os << "LGCoor: " << node.lgCoor << std::endl;
    os << "W: " << node.w << std::endl;
    os << "H: " << node.h << std::endl;
    os << "Displacement: " << node.getDisplacement() << std::endl;
    return os;
}