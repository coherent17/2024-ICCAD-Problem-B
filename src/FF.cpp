#include "FF.h"

FF::FF() : Instance(){
    ffIdx = UNSET_IDX;
    clusterIdx = UNSET_IDX;
    coor = {0, 0};
    bandwidth = MAX_BANDWIDTH;
    isShifting = true;
    prevStage = {nullptr, nullptr, ""};
    prevInstance = {nullptr, ""};
}

FF::FF(int size) : Instance(), clusterFF(size, nullptr){
    ffIdx = UNSET_IDX;
    clusterIdx = UNSET_IDX;
    coor = {0, 0};
    bandwidth = MAX_BANDWIDTH;
    isShifting = true;
    prevStage = {nullptr, nullptr, ""};
    prevInstance = {nullptr, ""};
}

FF::~FF(){}

// Setter
void FF::setTimingSlack(const std::string &pinName, double slack){
    TimingSlack[pinName] = slack;
}

void FF::addClusterFF(FF* inputFF, int slot){
    assert(clusterFF[slot] == nullptr && "slot already has FF");
    clusterFF[slot] = inputFF;
}

void FF::setFFIdx(int ffIdx){
    this->ffIdx = ffIdx;
}

void FF::setClusterIdx(int clusterIdx){
    this->clusterIdx = clusterIdx;
}

void FF::setNewCoor(const Coor &coor){
    this->newCoor = coor;
}

void FF::setBandwidth(){
    if(getNeighborSize() > BANDWIDTH_SELECTION_NEIGHBOR){
        bandwidth = NeighborFFs[BANDWIDTH_SELECTION_NEIGHBOR].second;
    }
    else{
        bandwidth = NeighborFFs.back().second;
    }

    if(bandwidth > MAX_SQUARE_DISPLACEMENT){
        bandwidth = MAX_BANDWIDTH;
    }
    else{
        bandwidth = std::sqrt(bandwidth);
    }
}

void FF::addNeighbor(int ffIdx, double euclidean_distance){
    NeighborFFs.push_back({ffIdx, euclidean_distance});
}

void FF::setIsShifting(bool shift){
    this->isShifting = shift;
}

void FF::setPrevStage(PrevStage inputStage){
    this->prevStage = inputStage;
}

void FF::setPrevInstance(std::pair<Instance*, std::string> inputInstance){
    this->prevInstance = inputInstance;
}

void FF::addNextStage(NextStage input){
    this->nextStage.push_back(input);
}

void FF::setPhysicalFF(FF* targetFF){
    this->physicalFF = targetFF;
}

void FF::setOriginalCoor(const Coor& coorD, const Coor& coorQ){
    this->originalD = coorD;
    this->originalQ = coorQ;
}

void FF::setOriginalQpinDelay(double in){
    this->originalQpinDelay = in;
}

// Getter
double FF::getTimingSlack(const std::string &pinName)const{
    auto it = TimingSlack.find(pinName);
    if (it == TimingSlack.end()) {
        throw std::out_of_range("Pin name not found");
    }
    return it->second;
}

int FF::getFFIdx()const{
    return ffIdx;
}

bool FF::getIsCluster()const{
    return clusterIdx != UNSET_IDX;
}

int FF::getClusterIdx()const{
    return clusterIdx;
}

Coor FF::getNewCoor()const{
    return newCoor;
}

double FF::getBandwidth()const{
    return bandwidth;
}

std::pair<int, double> FF::getNeighbor(int idx)const{
    return NeighborFFs[idx];
}

int FF::getNeighborSize()const{
    return NeighborFFs.size();
}

bool FF::getIsShifting()const{
    return isShifting;
}

PrevStage FF::getPrevStage()const{
    return this->prevStage;
}

std::pair<Instance*, std::string> FF::getPrevInstance()const{
    return this->prevInstance;
}

std::vector<NextStage> FF::getNextStage()const{
    return this->nextStage;
}

Coor FF::getOriginalD()const{
    return originalD;
}

Coor FF::getOriginalQ()const{
    return originalQ;
}

double FF::getOriginalQpinDelay()const{
    return originalQpinDelay;
}

void FF::sortNeighbors(){
    auto FFcmp = [](const std::pair<int, double> &neighbor1, const std::pair<int, double> &neighbor2){
        return neighbor1.second < neighbor2.second;
    };
    std::sort(NeighborFFs.begin(), NeighborFFs.end(), FFcmp);
}

double FF::shift(const Manager &mgr){
    double x_shift = 0;
    double y_shift = 0;
    double scale_factor = 0;
    for(size_t i = 0; i < NeighborFFs.size(); i++){
        FF *ffneighbor = mgr.FFs[NeighborFFs[i].first];
        double bandwidth_i = ffneighbor->getBandwidth();
        double weight = GaussianKernel(newCoor, ffneighbor->getCoor(), bandwidth_i) / std::pow(bandwidth_i, 4);
        // DEBUG_MSG(weight)
        x_shift += ffneighbor->getCoor().x * weight;
        y_shift += ffneighbor->getCoor().y * weight;
        scale_factor += weight;
    }
    assert(std::isnormal(scale_factor));
    x_shift /= scale_factor;
    y_shift /= scale_factor;
    double euclidean_distance = std::sqrt(SquareEuclideanDistance({x_shift, y_shift}, newCoor));
    setNewCoor({x_shift, y_shift});
    return euclidean_distance;
}

std::ostream &operator<<(std::ostream &os, const FF &ff){
    os << "Instance Name: " << ff.instanceName << std::endl;
    os << "Coor: " << ff.coor << std::endl;
    os << "CellName: " << ff.getCell()->getCellName() << std::endl;
    for(auto &pair : ff.TimingSlack)
        os << "Pin[" << pair.first << "] Slack: " << pair.second << std::endl;
    return os;
}
