#include "Node.h"
#include <cfloat>

Node::Node(){
    name = "";
    gpCoor = {0, 0};
    lgCoor = {DBL_MAX, DBL_MAX};
    w = 0;
    h = 0;
    weight = 0;
    isPlace = false;
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

void Node::setW(double w){
    this->w = w;
}

void Node::setH(double h){
    this->h = h;
}

void Node::setWeight(double weight){
    this->weight = weight;
}

void Node::setIsPlace(bool isPlace){
    this->isPlace = isPlace;
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

double Node::getW()const{
    return w;
}

double Node::getH()const{
    return h;
}

double Node::getWeight()const{
    return weight;
}

bool Node::getIsPlace()const{
    return isPlace;
}

double Node::getDisplacement(const Coor &candidateCoor)const{
    return std::sqrt(std::pow(gpCoor.x - candidateCoor.x, 2) + std::pow(gpCoor.y - candidateCoor.y, 2));
}

std::ostream &operator<<(std::ostream &os, const Node &node){
    os << "GPCoor" << node.gpCoor << std::endl;
    os << "W: " << node.w << std::endl;
    os << "H: " << node.h << std::endl;
    return os;
}