#include "Instance.h"

Instance::Instance() : 
    instanceName(""),
    cellName(""),
    coor(0, 0),
    cell(nullptr),
    largetInput(nullptr),
    largestOutput(nullptr)
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

void Instance::setLargestInput(Instance *input){
    this->largetInput = input;
}

void Instance::setLargestOutput(Instance *output){
    this->largestOutput = output;
}

void Instance::addInput(const std::string& input){
    this->inputInstances.push_back(input);
}

void Instance::addOutput(const std::string& output){
    this->outputInstances.push_back(output);
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

const Instance* Instance::getLargestInput()const{
    return (this->largetInput);
}

const Instance* Instance::getLargestOutput()const{
    return (this->largestOutput);
}

const std::vector<std::string>& Instance::getInputInstances()const{
    return inputInstances;
}

const std::vector<std::string>& Instance::getOutputInstances()const{
    return outputInstances;
}