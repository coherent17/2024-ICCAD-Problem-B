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
    preprocessor(nullptr)
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
    preprocessor = new Preprocess(*this);
    preprocessor->run();
    // delete all FF before preprocess
    originalFF_Map = FF_Map;
    FF_Map.clear();
    // assign new FF after debank and optimal location to FF_Map and FFs
    const std::unordered_map<std::string, FF*>& FF_list = preprocessor->getFFList();
    for(const auto& ff_m : FF_list){
        FF* newFF = getNewFF();
        FF* curFF = ff_m.second;
        Coor coor = curFF->getNewCoor();
        std::string instanceName = getNewFFName("FF_1_");
        int clkIdx = curFF->getClkIdx();
        const Cell* cell = curFF->getCell();
        newFF->setInstanceName(instanceName);
        newFF->setCoor(coor);
        newFF->setNewCoor(coor);
        newFF->setClkIdx(clkIdx);
        newFF->setCell(cell);
        newFF->setCellName(curFF->getCellName());
        newFF->setClusterSize(1);
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

void Manager::postBankingOptimize(){
    postBankingOptimizer postOptimize(*this);
    postOptimize.run();
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
    int clkIdx = FFToBank[0]->getClkIdx();
    for(auto& MBFF : FFToBank){
        assert(clkIdx == MBFF->getClkIdx() && "different clk cannot be banked together");
        std::vector<FF*>& clusterFF = MBFF->getClusterFF();
        for(auto& ff : clusterFF){
            FFs[bit] = ff;
            bit++;
        }
    }

    // delete all MBFF to be cluster from FF_Map
    for(auto& MBFF : FFToBank){
        FF_Map.erase(MBFF->getInstanceName());
        deleteFF(MBFF);
    }

    // assign new FF
    assert(bit == bankCellType->getBits() && "Floating input is allowed???");
    FF* newFF = getNewFF();
    std::string newName = getNewFFName("FF_" + std::to_string(bit) + "_");
    newFF->setInstanceName(newName);
    newFF->setCoor(newbankCoor);
    newFF->setNewCoor(newbankCoor);
    newFF->setCell(bankCellType);
    newFF->setCellName(bankCellType->getCellName());
    newFF->setClusterSize(bit);
    newFF->setClkIdx(clkIdx);
    FF_Map[newName] = newFF;


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
    int clkIdx = MBFF->getClkIdx();
    for(auto& ff : clusterFF){
        FF* newFF = getNewFF();
        // use coor for same D pin coor
        Coor coor = MBFF->getCoor() + MBFF->getPinCoor("D" + std::to_string(slot)) - debankCellType->getPinCoor("D");
        std::string instanceName = getNewFFName("FF_1_");
        newFF->setInstanceName(instanceName);
        newFF->setCoor(coor);
        newFF->setNewCoor(coor);
        newFF->setCell(debankCellType);
        newFF->setCellName(debankCellType->getCellName());
        newFF->setClusterSize(1);
        newFF->addClusterFF(ff, 0);
        newFF->setClkIdx(clkIdx);
        ff->setPhysicalFF(newFF, 0);

        FF_Map[instanceName] = newFF;
        slot++;
        outputFF.push_back(newFF);
    }

    FF_Map.erase(MBFF->getInstanceName());
    deleteFF(MBFF);

    return outputFF;
}

void Manager::getNS(double& TNS, double& WNS, bool show){
    TNS = 0;
    WNS = 0;
    for(auto& FF_m : FF_Map){
        double curTNS, curWNS;
        FF_m.second->getNS(curTNS, curWNS);
        TNS += curTNS;
        WNS = std::max(WNS, curWNS);
    }
    if(show){
        std::cout << "\tWorst negative slack : " << WNS << std::endl;
        std::cout << "\tTotal negative slack : " << TNS << std::endl << std::endl;
    }
}

double Manager::getTNS(){
    double TNS = 0;
    for(auto& FF_m : FF_Map){
        TNS += FF_m.second->getTNS();
    }
    return TNS;
}

double Manager::getWNS(){
    double WNS = 0;
    for(auto& FF_m : FF_Map){
        WNS = std::max(WNS, FF_m.second->getWNS());
    }
    return WNS;
}

void Manager::showNS(){
    double _0, _1;
    this->getNS(_0, _1, true);
}

FF* Manager::getNewFF(){
    if(FFGarbageCollector.empty()){
        FF* temp = new FF;
        return temp;
    }
    else{
        FF* temp = FFGarbageCollector.front();
        FFGarbageCollector.pop();
        return temp;
    }
}

void Manager::deleteFF(FF* in){
    in->clear();
    FFGarbageCollector.push(in);
}

// the cost function without evaluate the bin density
double Manager::getOverallCost(bool verbose){
    std::cout << "[Cost Evaluation]" << std::endl;
    double TNS_cost = 0;
    double Power_cost = 0;
    double Area_cost = 0;
    double Bin_cost = 0;
    for(const auto & ff_pair : FF_Map){
        double curTNS = ff_pair.second->getTNS();
        TNS_cost += alpha * (curTNS);
        Power_cost += beta * ff_pair.second->getCell()->getGatePower();
        Area_cost += gamma * (ff_pair.second->getCell()->getW() * ff_pair.second->getCell()->getH());
    }

    // check for the bin density
    if(!SKIP_BIN_CALC){
        int numBins = 0;
        int numViolationBins = 0;
        int DieStartX = die.getDieOrigin().x;
        int DieStartY = die.getDieOrigin().y;
        int DieEndX = die.getDieBorder().x;
        int DieEndY = die.getDieBorder().y;
        int BinW = die.getBinWidth();
        int BinH = die.getBinHeight();
        #pragma omp parallel for reduction(+:numBins, numViolationBins) collapse(2)
        for(int _x = DieStartX; _x < DieEndX; _x += BinW){
            for(int _y = DieStartY; _y < DieEndY; _y += BinH){
                numBins++;
                double area = 0;
                for(const auto &ff : FF_Map){
                    if(IsOverlap(Coor(_x, _y), die.getBinWidth(), die.getBinHeight(), ff.second->getNewCoor(), ff.second->getW(), ff.second->getH())){
                        area += ff.second->getW() * ff.second->getH();
                    }
                }

                for(const auto &gate : Gate_Map){
                    if(IsOverlap(Coor(_x, _y), die.getBinWidth(), die.getBinHeight(), gate.second->getCoor(), gate.second->getW(), gate.second->getH())){
                        area += gate.second->getW() * gate.second->getH();
                    }
                }
                
                // check if over bin max util
                if(area / (die.getBinWidth() * die.getBinHeight()) > die.getBinMaxUtil()){
                    numViolationBins++;
                }
            }
        }
        Bin_cost = lambda * numViolationBins;
    }
    else{
        std::cout << "[Warning] Skip bin density calculation!" << std::endl;
    }


    double cost = TNS_cost + Power_cost + Area_cost + Bin_cost;
    double TNS_percentage = TNS_cost / cost * 100;
    double Power_percentage = Power_cost / cost * 100;
    double Area_percentage = Area_cost / cost * 100;
    double Bin_percentage = Bin_cost / cost * 100;
    if(verbose){
        size_t numAfterDot = 4;
        std::vector<std::string> header = {"Cost", "Weight", "Value", "Percentage(%)"};
        std::vector<std::vector<std::string>> rows = {
            {"TNS", toStringWithPrecision(alpha, numAfterDot), toStringWithPrecision(TNS_cost, numAfterDot), toStringWithPrecision(TNS_percentage, numAfterDot) + "(%)"},
            {"Power", toStringWithPrecision(beta, numAfterDot), toStringWithPrecision(Power_cost, numAfterDot), toStringWithPrecision(Power_percentage, numAfterDot) + "(%)"},
            {"Area", toStringWithPrecision(gamma, numAfterDot), toStringWithPrecision(Area_cost, numAfterDot), toStringWithPrecision(Area_percentage, numAfterDot) + "(%)"},
            {"Bin", toStringWithPrecision(lambda, numAfterDot), toStringWithPrecision(Bin_cost, numAfterDot), toStringWithPrecision(Bin_percentage, numAfterDot) + "(%)"},
            {"Total", "-", toStringWithPrecision(cost, numAfterDot), "100.00(%)"},
            {"WNS", toStringWithPrecision(getWNS(), numAfterDot), "TNS", toStringWithPrecision(getTNS(), numAfterDot)}
        };

        PrettyTable pt;
        pt.AddHeader(header);
		pt.AddRows(rows);
        pt.SetAlign(PrettyTable::Align::Internal);
		std::cout << pt << std::endl;
    }
    return cost;
}

void Manager::libScoring(){
    for(auto &pair: Bit_FF_Map){
        std::vector<Cell *> &cell_vector = pair.second;
        for(size_t i = 0; i < cell_vector.size(); i++){
            double area = cell_vector[i]->getArea();
            double score = alpha*cell_vector[i]->getQpinDelay() + beta*cell_vector[i]->getGatePower() + gamma*area;
            cell_vector[i]->setScore(score);
        }
        sortCell(cell_vector);
        //DEBUG
        // for(size_t i = 0; i < pair.second.size(); i++){
        //     std::cout << pair.second[i]->getCellName() << ": " << pair.second[i]->getScore() << std::endl;
        // }
    }
    // DEBUG
    // std::map<int, std::vector<Cell *>> bit_map(Bit_FF_Map.begin(), Bit_FF_Map.end());
    // for(auto &pair: bit_map){
    //     std::cout << pair.second[0]->getCellName() << ": " << pair.second[0]->getScore() << std::endl; 
    // }

}

void Manager::sortCell(std::vector<Cell *> &cell_vector){
    auto scoreCmp = [](const Cell * cell1, const Cell * cell2){
        return cell1->getScore() < cell2->getScore();
    };
    std::sort(cell_vector.begin(), cell_vector.end(), scoreCmp);
}