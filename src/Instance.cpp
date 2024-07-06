#include "Instance.h"

Instance::Instance() : 
    instanceName(""),
    cellName(""),
    coor(0, 0),
    cell(nullptr),
    largestInput(nullptr),
    nextStageFF(nullptr),
    largestOutput({nullptr, ""})
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
    this->largestInput = input;
}

void Instance::setLargestOutput(Instance *output, const std::string& pinName){
    this->largestOutput = {output, pinName};
}

void Instance::setNextStageFF(Instance *input){
    this->nextStageFF = input;
}

void Instance::addInput(const std::string& input, const std::string& pinName){
    this->inputInstances.push_back({input, pinName});
}

void Instance::addOutput(const std::string& output, const std::string& pinName){
    this->outputInstances.push_back({output, pinName});
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
    return (this->largestInput);
}

const std::pair<Instance*, std::string> Instance::getLargestOutput()const{
    return (this->largestOutput);
}

const Instance* Instance::getNextStageFF()const{
    return this->nextStageFF;
}

const std::vector<std::pair<std::string, std::string>>& Instance::getInputInstances()const{
    return inputInstances;
}

const std::vector<std::pair<std::string, std::string>>& Instance::getOutputInstances()const{
    return outputInstances;
}