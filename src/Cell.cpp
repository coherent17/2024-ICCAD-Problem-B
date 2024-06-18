#include "Cell.h"

// constructor & destructor
Cell::Cell(){
    this->bits = 0;
    this->cellName = "";
    this->isFF = false;
    this->w = 0;
    this->h = 0;
    this->pinCount = 0;
}

Cell::~Cell(){
    ;
}

// setter
void Cell::setCellName(const string &cellName){
    this->cellName = cellName;
}

void Cell::setIsFF(bool isFF){
    this->isFF = isFF;
}

void Cell::setBits(int bits){
    this->bits = bits;
}

void Cell::setW(double w){
    this->w = w;
}

void Cell::setH(double h){
    this->h = h;
}

void Cell::setPinCount(int pinCount){
    this->pinCount = pinCount;
}

void Cell::addPinCoor(const string &pinName, Coor &coor){
    this->pinCoorMap[pinName] = coor;
}

void Cell::addPinName(const string &pinName){
    this->pinName.push_back(pinName);
}

void Cell::setQpinDelay(double QpinDelay){
    this->QpinDelay = QpinDelay;
}

void Cell::setGatePower(double GatePower){
    this->GatePower = GatePower;
}

// getter
const string &Cell::getCellName()const{
    return cellName;
}


bool Cell::getisFF()const{
    return isFF;
}

int Cell::getBit()const{
    return bits;
}

double Cell::getW()const{
    return w;
}

double Cell::getH()const{
    return h;
}

int Cell::getPinCount()const{
    return pinCount;
}

const string &Cell::getPinName(const int& i) const{
    return this->pinName[i];
}

const Coor &Cell::getPinCoor(const string &pinName)const{
    auto it = pinCoorMap.find(pinName);
    assert(it != pinCoorMap.end());
    return it->second;
}

double Cell::getQpinDelay() const{
    assert(isFF);
    return QpinDelay;
}

double Cell::getGatePower() const{
    assert(isFF);
    return GatePower;
}

ostream &operator<<(ostream &out, const Cell &c){
    if(c.isFF){
        out << "CellName: " << c.cellName << endl;
        out << "IsFF: " << c.isFF << endl;
        out << "Number of bits: " << c.bits << endl;
        out << "Width: " << c.w << endl;
        out << "Height: " << c.h << endl;
        out << "Pin Count: " << c.pinCount << endl;
        out << "QpinDelay: " << c.QpinDelay << endl;
        out << "GatePower: " << c.GatePower << endl;
    }
    else{
        out << "CellName: " << c.cellName << endl;
        out << "IsFF: " << c.isFF << endl;
        out << "Width: " << c.w << endl;
        out << "Height: " << c.h << endl;
        out << "Pin Count: " << c.pinCount << endl;
    }

    for(const auto &pair : c.pinCoorMap){
        out << "\t" << pair.first << " " << pair.second << endl;
    }
    return out;
}

Instance::Instance(){
    ;
}

Instance::~Instance(){
    ;
}

// setter
void Instance::setInstanceName(const string &instanceName){
    this->instanceName = instanceName;
}

void Instance::setCellName(const string &cellName){
    this->cellName = cellName;
}

void Instance::setCoor(Coor &coor){
    this->coor = coor;
}

void Instance::setCell(const Cell &cell){
    this->cell = cell;
}

void Instance::setLargestInput(Instance* input){
    this->largetInput = input;
}

void Instance::setLargestOutput(Instance* output){
    this->largestOutput = output;
}

void Instance::addInput(const string& s){
    this->inputInstance.push_back(s);
}

void Instance::addOutput(const string& s){
    this->outputInstance.push_back(s);
}

// getter
const string &Instance::getInstanceName()const{
    return instanceName;
}

const string &Instance::getCellName()const{
    return cellName;
}

const Cell& Instance::getCell()const{
    return cell;
}

double Instance::getW()const{
    return cell.getW();
}

double Instance::getH()const{
    return cell.getH();
}


Coor Instance::getCoor()const{
    return coor;
}

int Instance::getPinCount()const{
    return cell.getPinCount();
}

const Coor &Instance::getPinCoor(const string &pinName)const{
    return cell.getPinCoor(pinName);
}

const Instance* Instance::getLargestInput()const{
    return (this->largetInput);
}

const Instance* Instance::getLargestOutput()const{
    return (this->largestOutput);
}

const vector<string>& Instance::getInput()const{
    return inputInstance;
}

const vector<string>& Instance::getOutput()const{
    return outputInstance;
}