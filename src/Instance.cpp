#include "Instance.h"

Instance::Instance() : 
    instanceName(""),
    cellName(""),
    coor(0, 0),
    cell(nullptr)
    {}

Instance::~Instance(){}

// Setters
void Instance::setInstanceName(const std::string &instanceName){
    this->instanceName = instanceName;
}

void Instance::setCellName(const std::string &cellName){
    this->cellName = cellName;
}

void Instance::setCoor(const Coor &coor){
    this->coor = coor;
}

void Instance::setCell(const Cell *cell){
    this->cell = cell;
}

void Instance::addInput(const std::string& pinName, const std::string& input, const std::string& instPinName){
    this->inputInstances[pinName].push_back({input, instPinName});
}

void Instance::addOutput(const std::string& pinName, const std::string& output, const std::string& instPinName){
    this->outputInstances[pinName].push_back({output, instPinName});
}

// Getters
const std::string &Instance::getInstanceName()const{
    return instanceName;
}

const std::string &Instance::getCellName()const{
    return cellName;
}

Coor Instance::getCoor()const{
    return coor;
}

const Cell *Instance::getCell()const{
    return cell;
}

double Instance::getW()const{
    return cell->getW();
}

double Instance::getH()const{
    return cell->getH();
}

int Instance::getPinCount()const{
    return cell->getPinCount();
}

const Coor &Instance::getPinCoor(const std::string &pinName)const{
    return cell->getPinCoor(pinName);
}

std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>>& Instance::getInputInstances(){
    return inputInstances;
}

std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>>& Instance::getOutputInstances(){
    return outputInstances;
}