#include "Manager.h"
Manager::Manager():
    alpha(0),
    beta(0),
    gamma(0),
    lambda(0),
    DisplacementDelay(0),
    NumInput(0),
    NumOutput(0),
    MaxBit(0),
    NumInstances(0),
    NumNets(0),
    preprocessor(new Preprocess(*this))
    {}

Manager::~Manager(){
    for(auto &pair : FF_Map){
        delete pair.second;
    }
    FF_Map.clear();

    for(auto &pair : Gate_Map){
        delete pair.second;
    }
    Gate_Map.clear();
}

void Manager::parse(const std::string &filename){
    Parser parser(filename);
    parser.parse(*this);
}

void Manager::preprocess(){
    preprocessor->run();
    // delete all FF before preprocess
    originalFF_Map = FF_Map;
    FF_Map.clear();
    // assign new FF after debank and optimal location to FF_Map and FFs
    const std::unordered_map<std::string, FF*>& FF_list = preprocessor->getFFList();
    for(const auto& ff_m : FF_list){
        FF* newFF = new FF(1);
        FF* curFF = ff_m.second;
        Coor coor = curFF->getNewCoor();
        std::string instanceName = getNewFFName("FF_1_");
        const Cell* cell = curFF->getCell();
        newFF->setInstanceName(instanceName);
        newFF->setCoor(coor);
        newFF->setNewCoor(coor);
        newFF->setCell(cell);
        newFF->setCellName(curFF->getCellName());
        newFF->addClusterFF(curFF, 0);

        curFF->setPhysicalFF(newFF, 0);

        FF_Map[instanceName] = newFF;
    }
}

void Manager::meanshift(){
    // do graceful meanshift clustering
    std::cout << "do graceful meanshift clustering..." << std::endl;
    MeanShift meanshift;
    meanshift.run(*this);
}

void Manager::banking(){
    Banking banking(*this);
    banking.run();
}

void Manager::legalize(){
    // do abacus legalize algorithm
    Legalizer legalizer(*this);
    legalizer.run();
}

void Manager::dump(const std::string &filename){
    Dumper dumper(filename);
    dumper.dump(*this);
}

void Manager::dumpVisual(const std::string &filename){
    std::ofstream fout;
    fout.open(filename.c_str());
    assert(fout.good());

    fout << "DieSize " << die.getDieOrigin().x << " " << die.getDieOrigin().y << " " << die.getDieBorder().x << " " << die.getDieBorder().y << std::endl;

    fout << "NumInput " << Input_Map.size() << std::endl;
    std::map<std::string, Coor> input_map(Input_Map.begin(), Input_Map.end());
    for(const auto &pair: input_map){
        fout << "Input " << pair.first << " " << pair.second.x << " " << pair.second.y << std::endl;
    }
    
    fout << "NumOutput " << Output_Map.size() << std::endl;
    std::map<std::string, Coor> output_map(Output_Map.begin(), Output_Map.end());
    for(const auto &pair: output_map){
        fout << "Output " << pair.first << " " << pair.second.x << " " << pair.second.y << std::endl;
    }


    // for cell library
    std::unordered_map<std::string, Cell *> cellMap = cell_library.getCellMap();
    for(const auto &pair: cellMap){
        if(pair.second->getType() == Cell_Type::FF){
            fout << "FlipFlop " << pair.second->getBits() << " " << pair.second->getCellName() << " " << pair.second->getW() << " " << pair.second->getH() << " " << pair.second->getPinCount() << std::endl;
            std::unordered_map<std::string, Coor> pinCoorMap= pair.second->getPinCoorMap();
            for(const auto &p : pinCoorMap){
                fout << "Pin " << p.first << " " << p.second.x << " " << p.second.y << std::endl;
            }
        }
        else if(pair.second->getType() == Cell_Type::Gate){
            fout << "Gate " << pair.second->getCellName() << " " << pair.second->getW() << " " << pair.second->getH() << " " << pair.second->getPinCount() << std::endl;
            std::unordered_map<std::string, Coor> pinCoorMap= pair.second->getPinCoorMap();
            for(const auto &p : pinCoorMap){
                fout << "Pin " << p.first << " " << p.second.x << " " << p.second.y << std::endl;
            }
        }
        else{
            abort();
        }
    }

    fout << "NumInstances " << FF_Map.size() + Gate_Map.size() << std::endl;
    std::map<std::string, FF *> ff_map(FF_Map.begin(), FF_Map.end());
    for(const auto &pair: ff_map){
        fout << "Inst " << pair.first << " " << pair.second->getCellName() << " " << pair.second->getNewCoor().x << " " << pair.second->getNewCoor().y << std::endl;
    }
    std::map<std::string, Gate *> gate_map(Gate_Map.begin(), Gate_Map.end());
    for(const auto &pair: gate_map){
        fout << "Inst " << pair.first << " " << pair.second->getCellName() << " " << pair.second->getCoor().x << " " << pair.second->getCoor().y << std::endl;
    }

    std::map<std::string, Net> net_map(Net_Map.begin(), Net_Map.end());
    for(const auto &pair: net_map){
        int pinCout = pair.second.getNumPins();
        fout << "Net " << pair.second.getNetName() << " " << pinCout << std::endl;
        for(int i = 0; i < pinCout; i++){
            Pin pin = pair.second.getPin(i);
            fout << "Pin ";
            if(!pin.getIsIOPin()){
                fout << pin.getInstanceName() << "/";
            }
            fout << pin.getPinName() << std::endl;
        }
    }

    fout << "BinWidth " << die.getBinWidth() << std::endl;
    fout << "BinHeight " << die.getBinHeight() << std::endl;
    fout << "BinMaxUtil " << die.getBinMaxUtil() << std::endl;
    std::vector<PlacementRow> pr = die.getPlacementRows();
    for(size_t i = 0; i < pr.size(); i++){
        fout << "PlacementRows " << pr[i].startCoor.x << " " << pr[i].startCoor.y << " " << pr[i].siteWidth << " " << pr[i].siteHeight << " " << pr[i].NumOfSites << std::endl;
    }
    fout.close();
}

void Manager::print(){
    std::cout << alpha << " " << beta << " " << gamma << " " << lambda << std::endl;
    std::cout << "#################### Die Info ##################" << std::endl;
    std::cout << die << std::endl;

    std::cout << "#################### IO Info ##################" << std::endl;
    for(const auto &pair : Input_Map){
        std::cout << pair.first << ":" << pair.second << std::endl;
    }
    for(const auto &pair : Output_Map){
        std::cout << pair.first << ":" << pair.second << std::endl;
    }

    std::cout << "#################### Cell Library ##################" << std::endl;
    std::cout << cell_library << std::endl;

    std::cout << "#################### FF Instance ##################" << std::endl;
    for(const auto &pair: FF_Map){
        std::cout << *pair.second << std::endl;
    }

    std::cout << "#################### Gate Instance ##################" << std::endl;
    for(const auto &pair: Gate_Map){
        std::cout << *pair.second << std::endl;
    }

    std::cout << "#################### Netlist ##################" << std::endl;
    for(const auto &pair: Net_Map){
        std::cout << pair.second << std::endl;
    }

    std::cout << "#################### After MeanShift ##################" << std::endl;
    for(const auto &pair: FF_Map){
        std::cout << pair.second->getCoor() << pair.second->getNewCoor() << std::endl;
    }
}

bool Manager::isIOPin(const std::string &pinName){
    if(Input_Map.find(pinName) != Input_Map.end()) return true;
    if(Output_Map.find(pinName) != Output_Map.end()) return true;
    return false;
}

std::string Manager::getNewFFName(const std::string& prefix){
    int count = name_record[prefix];
    assert("number of FF exceed INT_MAX, pls modify counter datatype" && count != INT_MAX);
    name_record[prefix]++;
    return prefix + std::to_string(count);
}

struct ComparePairs {
  bool operator()(const std::pair<double, FF*>& a, const std::pair<double, FF*>& b) const {
    // Priority based on the first element (ascending order for min heap)
    return a.first > b.first;
  }
};

FF* Manager::bankFF(Coor newbankCoor, Cell* bankCellType, std::vector<FF*> FFToBank){
    // get all FF to be bank
    std::vector<FF*> FFs(bankCellType->getBits());
    int bit = 0;
    for(auto& MBFF : FFToBank){
        std::vector<FF*>& clusterFF = MBFF->getClusterFF();
        for(auto& ff : clusterFF){
            FFs[bit] = ff;
            bit++;
        }
    }

    // delete all MBFF to be cluster from FF_Map
    for(auto& MBFF : FFToBank){
        FF_Map.erase(MBFF->getInstanceName());
        delete MBFF;
    }

    // assign new FF
    assert(bit == bankCellType->getBits() && "Floating input is allowed???");
    FF* newFF = new FF(bit);
    newFF->setInstanceName(getNewFFName("FF_" + std::to_string(bit) + "_"));
    newFF->setCoor(newbankCoor);
    newFF->setNewCoor(newbankCoor);
    newFF->setCell(bankCellType);
    newFF->setCellName(bankCellType->getCellName());
    FF_Map[newFF->getInstanceName()] = newFF;


    // Greedy from smallest slack
    std::priority_queue<std::pair<double, FF*>, std::vector<std::pair<double, FF*>>, ComparePairs> pq;
    for(auto& ff : FFs){
        pq.push({ff->getTimingSlack("D"), ff});
    }
    // assign to smallest HPWL D slot
    std::vector<bool> slotEmpty(bit, true);
    while(!pq.empty()){
        FF* curFF = pq.top().second;
        pq.pop();
        int slot = -1;
        double bestHPWL = DBL_MAX;
        for(int i=0;i<bit;i++){
            if(slotEmpty[i]){
                double curHPWL = HPWL(curFF->getOriginalD(), newbankCoor + bankCellType->getPinCoor("D" + std::to_string(i)));
                if(curHPWL < bestHPWL){
                    bestHPWL = curHPWL;
                    slot = i;
                }
            }
        }

        assert(slot != -1 && "why you can't findout your best location, you are looser");
        slotEmpty[slot] = false;
        newFF->addClusterFF(curFF, slot);
        curFF->setPhysicalFF(newFF, slot);
    }

    return newFF;
}

std::vector<FF*> Manager::debankFF(FF* MBFF, Cell* debankCellType){
    std::vector<FF*> outputFF;
    std::vector<FF*>& clusterFF = MBFF->getClusterFF();
    int slot = 0;
    for(auto& ff : clusterFF){
        FF* newFF = new FF(1);
        // use coor for same D pin coor
        Coor coor = MBFF->getCoor() + MBFF->getPinCoor("D" + std::to_string(slot)) - debankCellType->getPinCoor("D");
        std::string instanceName = getNewFFName("FF_1_");
        newFF->setInstanceName(instanceName);
        newFF->setCoor(coor);
        newFF->setNewCoor(coor);
        newFF->setCell(debankCellType);
        newFF->setCellName(debankCellType->getCellName());
        newFF->addClusterFF(ff, 0);

        ff->setPhysicalFF(newFF, 0);

        FF_Map[instanceName] = newFF;
        slot++;
        outputFF.push_back(newFF);
    }

    FF_Map.erase(MBFF->getInstanceName());
    delete MBFF;

    return outputFF;
}

void Manager::getNS(double& TNS, double& WNS, bool show){
    TNS = 0;
    WNS = 0;
    for(auto& FF_m : FF_Map){
        double curTNS, curWNS;
        FF_m.second->updateSlack(*this);
        FF_m.second->getNS(curTNS, curWNS);
        TNS += curTNS;
        WNS = std::min(WNS, curWNS);
    }
    if(show){
        std::cout << "\tWorst negative slack : " << WNS << std::endl;
        std::cout << "\tTotal negative slack : " << TNS << std::endl << std::endl;
    }
}

void Manager::showNS(){
    double _0, _1;
    this->getNS(_0, _1, true);
}

// the cost function without evaluate the bin density
double Manager::getOverallCost(){
    double cost = 0;
    for(const auto & ff_pair : FF_Map){
        double curTNS, curWNS;
        ff_pair.second->updateSlack(*this);
        ff_pair.second->getNS(curTNS, curWNS);
        cost += alpha * curTNS + beta * ff_pair.second->getCell()->getGatePower() + gamma * (ff_pair.second->getCell()->getW() * ff_pair.second->getCell()->getH());
    }
    return cost;
}