#include "FF.h"

FF::FF() : Instance(){
    ffIdx = UNSET_IDX;
    clusterIdx = UNSET_IDX;
    coor = {0, 0};
    bandwidth = 0;
    isShifting = true;
    prevStage = {nullptr, nullptr, ""};
    prevInstance = {nullptr, ""};
}

FF::FF(int size) : Instance(), clusterFF(size, nullptr){
    ffIdx = UNSET_IDX;
    clusterIdx = UNSET_IDX;
    coor = {0, 0};
    bandwidth = 0;
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

void FF::setBandwidth(const Manager &mgr){
    if(getNeighborSize() > BANDWIDTH_SELECTION_NEIGHBOR){
        bandwidth = NeighborFFs[BANDWIDTH_SELECTION_NEIGHBOR].second;
    }
    else{
        bandwidth = NeighborFFs.back().second;
    }

    if(bandwidth > mgr.param.MAX_SQUARE_DISPLACEMENT){
        bandwidth = mgr.param.MAX_BANDWIDTH;
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

void FF::setPhysicalFF(FF* targetFF, int slot){
    this->physicalFF = targetFF;
    this->slot = slot;
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

std::vector<FF*>& FF::getClusterFF(){
    return this->clusterFF;
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

FF* FF::getPhysicalFF()const{
    return physicalFF;
}

int FF::getSlot()const{
    return slot;
}

void FF::sortNeighbors(){
    auto FFcmp = [](const std::pair<int, double> &neighbor1, const std::pair<int, double> &neighbor2){
        return neighbor1.second < neighbor2.second;
    };
    std::sort(NeighborFFs.begin(), NeighborFFs.end(), FFcmp);
}

double FF::shift(std::vector<FF *> &FFs){
    double x_shift = 0;
    double y_shift = 0;
    double scale_factor = 0;
    for(size_t i = 0; i < NeighborFFs.size(); i++){
        FF *ffneighbor = FFs[NeighborFFs[i].first];
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

void FF::getNS(double& TNS, double& WNS){
    TNS = 0;
    WNS = 0;
    for(const auto& slack_m : TimingSlack){
        if(slack_m.second < 0){
            TNS += slack_m.second;
            WNS = std::min(slack_m.second, WNS);
        }
    }
}

void FF::updateSlack(Manager& mgr){
    int curSlot = 0;
    for(auto& FF : clusterFF){
        double slack = FF->getSlack(mgr);
        if(clusterFF.size() == 1){
            TimingSlack["D"] = slack;
        }
        else{
            TimingSlack["D" + std::to_string(curSlot)] = slack;
        }
        curSlot++;
    }
}

std::ostream &operator<<(std::ostream &os, const FF &ff){
    os << "Instance Name: " << ff.instanceName << std::endl;
    os << "Coor: " << ff.coor << std::endl;
    os << "CellName: " << ff.getCell()->getCellName() << std::endl;
    for(auto &pair : ff.TimingSlack)
        os << "Pin[" << pair.first << "] Slack: " << pair.second << std::endl;
    return os;
}


double FF::getSlack(Manager& mgr){
    FF* cur_ff = this;
    std::unordered_map<std::string, FF*>& FF_list =  mgr.preprocessor->getFFList();
    // update slack for new location
    Coor newCoorD = this->physicalFF->getNewCoor() + this->physicalFF->getPinCoor("D" + this->getPhysicalPinName());
    double delta_hpwl = 0;
    double delta_q = 0; // delta q pin delay
    Coor inputCoor;
    // D pin delta HPWL
    if(cur_ff->getPrevInstance().first){
        std::string inputInstanceName = cur_ff->getPrevInstance().first->getInstanceName();
        std::string inputPinName = cur_ff->getPrevInstance().second;
        if(mgr.IO_Map.count(inputInstanceName)){
            inputCoor = mgr.IO_Map[inputInstanceName].getCoor();
            double old_hpwl = HPWL(inputCoor, cur_ff->getOriginalD());
            double new_hpwl = HPWL(inputCoor, newCoorD);
            delta_hpwl += old_hpwl - new_hpwl;
        }
        else if(mgr.Gate_Map.count(inputInstanceName)){
            inputCoor = mgr.Gate_Map[inputInstanceName]->getCoor() + mgr.Gate_Map[inputInstanceName]->getPinCoor(inputPinName);
            double old_hpwl = HPWL(inputCoor, cur_ff->getOriginalD());
            double new_hpwl = HPWL(inputCoor, newCoorD);
            delta_hpwl += old_hpwl - new_hpwl;
        }
        else{
            FF* inputFF = FF_list[inputInstanceName];
            inputCoor = inputFF->getOriginalQ();
            Coor newCoorQ = inputFF->physicalFF->getNewCoor() + inputFF->physicalFF->getPinCoor("Q" + inputFF->getPhysicalPinName());
            double old_hpwl = HPWL(inputCoor, cur_ff->getOriginalD());
            double new_hpwl = HPWL(newCoorQ, newCoorD);
            delta_hpwl += old_hpwl - new_hpwl;
        }
    }

    // Q pin delta HPWL (prev stage FFs Qpin)
    const PrevStage& prev = cur_ff->getPrevStage();
    if(prev.ff){
        std::string prevInstanceName = prev.ff->getInstanceName();
        Coor originalInput, newInput;
        if(FF_list.count(prevInstanceName)){
            FF* inputFF = FF_list[prevInstanceName];
            originalInput = inputFF->getOriginalQ();
            newInput = inputFF->physicalFF->getNewCoor() + inputFF->physicalFF->getPinCoor("Q" + inputFF->getPhysicalPinName());
            delta_q = inputFF->getOriginalQpinDelay() - inputFF->physicalFF->getCell()->getQpinDelay();
        }
        else{
            assert(0 && "Prev shold always be FF");
        }

        if(prev.outputGate){
            std::string outputInstanceName = prev.outputGate->getInstanceName();
            if(mgr.Gate_Map.count(outputInstanceName)){
                inputCoor = mgr.Gate_Map[outputInstanceName]->getCoor() + mgr.Gate_Map[outputInstanceName]->getPinCoor(prev.pinName);
                double old_hpwl = HPWL(inputCoor, originalInput);
                double new_hpwl = HPWL(inputCoor, newInput);
                delta_hpwl += old_hpwl - new_hpwl;
            }
            else{
                assert(0 && "prev output should always be std cell");
            }
        }
    }
    // get new slack
    double newSlack = cur_ff->getTimingSlack("D") + (delta_q) + mgr.DisplacementDelay * delta_hpwl;
    return newSlack;
}

std::string FF::getPhysicalPinName(){
    if(this->physicalFF->getCell()->getBits() == 1)
        return "";
    else
        return std::to_string(this->slot); 
}
