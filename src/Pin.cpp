#include "Pin.h"

Pin::Pin() : pinName(""), isIOPin(false), instanceName(""){}

Pin::~Pin(){}

// Setters
void Pin::setPinName(const std::string &pinName){
    this->pinName = pinName;
}

void Pin::setIsIOPin(bool isIOPin){
    this->isIOPin = isIOPin;
}

void Pin::setInstanceName(const std::string &instanceName){
    this->instanceName = instanceName;
}

// Getters
const std::string &Pin::getPinName()const{
    return this->pinName;
}

bool Pin::getIsIOPin()const{
    return this->isIOPin;
}

const std::string &Pin::getInstanceName()const{
    return instanceName;
}

std::ostream &operator<<(std::ostream &os, const Pin &pin){
    if(!pin.isIOPin){
        os << pin.instanceName << "/" << pin.pinName << " ";
    }
    else{
        os << pin.pinName << " ";
    }
    return os;
}