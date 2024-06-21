#include "Net.h"

Net::Net() : netName(""), numPins(0){}

Net::~Net(){}

// Setters
void Net::setNetName(const std::string &netName){
    this->netName = netName;
}

void Net::setNumPins(int numPins){
    this->numPins = numPins;
}

void Net::addPins(const Pin &pin){
    this->pins.push_back(pin);
}

// Getters
const std::string &Net::getNetName()const{
    return this->netName;
}

int Net::getNumPins()const{
    return this->numPins;
}

const Pin &Net::getPin(int pinIdx)const{
    return this->pins[pinIdx];
}

std::ostream &operator<<(std::ostream &os, const Net &net){
    os << "NetName: " << net.netName << std::endl;
    os << "NumPins: " << net.numPins << std::endl;
    for(size_t i = 0; i < net.pins.size(); i++){
        os << net.pins[i];
    }
    os << std::endl;
    return os;
}