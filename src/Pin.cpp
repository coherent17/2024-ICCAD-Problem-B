#include "Pin.h"

Pin::Pin(){
    ;
}

Pin::~Pin(){
    ;
}

// setter
void Pin::setPinName(const string &pinName){
    this->pinName = pinName;
}

void Pin::setIsIOPin(bool isIOPin){
    this->isIOPin = isIOPin;
}

void Pin::setInstanceName(const string &instanceName){
    this->instanceName = instanceName;
}

// getter
const string &Pin::getPinName()const{
    return this->pinName;
}

bool Pin::getIsIOPin()const{
    return this->isIOPin;
}

const string &Pin::getInstanceName()const{
    return instanceName;
}

ostream &operator<<(ostream &out, const Pin &pin){
    if(!pin.isIOPin){
        out << pin.instanceName << "/" << pin.pinName << " ";
    }
    else{
        out << pin.pinName << " ";
    }
    return out;
}