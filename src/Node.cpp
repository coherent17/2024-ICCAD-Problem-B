#include "Node.h"

Node::Node(){
    name = "";
    gpCoor = {0, 0};
    lgzCoor = {0, 0};
    w = 0;
    h = 0;
    weight = 0;
}

Node::~Node(){}

// Setters
void Node::setName(const std::string &name){
    this->name = name;
}

void Node::setGPCoor(const Coor &gpCoor){
    this->gpCoor = gpCoor;
}

void Node::setLGZCoor(const Coor &lgzCoor){
    this->lgzCoor = lgzCoor;
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

// Getters
const std::string &Node::getName()const{
    return name;
}

const Coor &Node::getGPCoor()const{
    return gpCoor;
}

const Coor &Node::getLHZCoor()const{
    return lgzCoor;
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