#include "Net.h"

Net::Net(){
    ;
}

Net::~Net(){
    ;
}

// setter
void Net::setNetName(const string &netName){
    this->netName = netName;
}

void Net::setNumPins(int numPins){
    this->numPins = numPins;
}

void Net::addPins(const Pin &pin){
    this->pins.push_back(pin);
}

// getter
const string &Net::getNetName()const{
    return this->netName;
}

int Net::getNumPins()const{
    return this->numPins;
}

const Pin& Net::getPin(int i)const{
    return this->pins[i];
}

ostream &operator<<(ostream &out, const Net &net){
    out << "NetName: " << net.netName << endl;
    out << "NumPins: " << net.numPins << endl;
    for(size_t i = 0; i < net.pins.size(); i++){
        out << net.pins[i];
    }
    out << endl;
    return out;
}