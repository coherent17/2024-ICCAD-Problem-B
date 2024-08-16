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
    NumNets(0)
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

    delete legalizer;
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
        Cell* cell = curFF->getCell();
        newFF->setInstanceName(instanceName);
        newFF->setCoor(coor);
        newFF->setNewCoor(coor);
        newFF->setClkIdx(clkIdx);
        newFF->setCell(cell);
        newFF->setClusterSize(1);
        newFF->addClusterFF(curFF, 0);
        newFF->setFixed(curFF->getFixed());
        curFF->setPhysicalFF(newFF, 0);

        FF_Map[instanceName] = newFF;
    }
}

void Manager::meanshift(){
    // do graceful meanshift clustering
    MeanShift meanshift;
    meanshift.run(*this);
}

void Manager::preLegalize(){
    legalizer = new Legalizer(*this);
    legalizer->initial();
    legalizer->run();
}

void Manager::banking(){
    legalizer = new Legalizer(*this);
    legalizer->initial();
    Banking banking(*this);
    banking.run();
}

void Manager::postBankingOptimize(){
    postBankingOptimizer postOptimize(*this);
    postOptimize.run();
}

void Manager::legalize(){
    legalizer->run();
}

void Manager::detailplacement(){
    DetailPlacement detailplacer(*this);
    detailplacer.run();
}

void Manager::checker(){
    Checker checker(*this);
    checker.run();
}

void Manager::dump(const std::string &filename, double prePlaceCost, double finalCost){
    DEBUG_MGR("Dump result ...");
    DEBUG_MGR("Input cost : " + std::to_string(prePlaceCost));
    DEBUG_MGR("Final cost : " + std::to_string(finalCost));
    Dumper dumper(filename);
    if(finalCost < prePlaceCost)
        dumper.dump(*this);
    else{
        DEBUG_MGR("[WARNING] worse performance after optimize, dump pre-place FF");
        DEBUG_MGR("[WARNING] worse performance after optimize, dump pre-place FF");
        DEBUG_MGR("[WARNING] worse performance after optimize, dump pre-place FF");
        FF_Map = originalFF_Map;    // retrieve input FF
        for(auto& ff : FF_Map){
            ff.second->setNewCoor(ff.second->getCoor());    
        }
        legalizer = new Legalizer(*this); // make sure result is legal
        legalizer->initial();
        legalizer->run();           
        this->getOverallCost(true); 
        dumper.dumpPrePlace(*this); // write result
    }
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
        fout << "Inst " << pair.first << " " << pair.second->getCell()->getCellName() << " " << pair.second->getNewCoor().x << " " << pair.second->getNewCoor().y << std::endl;
    }
    std::map<std::string, Gate *> gate_map(Gate_Map.begin(), Gate_Map.end());
    for(const auto &pair: gate_map){
        fout << "Inst " << pair.first << " " << pair.second->getCell()->getCellName() << " " << pair.second->getCoor().x << " " << pair.second->getCoor().y << std::endl;
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

/**
 * @brief Bank MBFF/single bit ff into MBFF
 * 
 * @param newbankCoor Merge MBFF coordinate
 * @param bankCellType Merge MBFF celltype
 * @param FFToBank All ffs needs to bank
 * @return FF* The pointer point to the merged MBFF
 * @todo add incremental banking??
 */
FF* Manager::bankFF(Coor newbankCoor, Cell* bankCellType, std::vector<FF*> FFToBank){
    // get all FF to be bank
    std::vector<FF*> FFs(bankCellType->getBits());
    int bit = 0;
    int clkIdx = FFToBank[0]->getClkIdx();
    
    // if the FFToBank exist MBFF
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
    newFF->setClusterSize(bit);
    newFF->setClkIdx(clkIdx);
    newFF->setFixed(false);
    FF_Map[newName] = newFF;

    if(bit == 1){ // bank single bit FF
        newFF->addClusterFF(FFs[0], 0);
        FFs[0]->setPhysicalFF(newFF, 0);
        return newFF;
    }

    for(size_t i=0;i<FFs.size();i++){
        FFs[i]->setPhysicalFF(newFF, i);
        newFF->addClusterFF(FFs[i], i);
    }
    assignSlot(newFF);
    return newFF;
}

/**
 * @brief For merged MBFF, try to assign slot based on the slack, do stable matching
 * 
 * @param newFF The merged MBFF
 */
void Manager::assignSlot(FF* newFF){
    int bit = newFF->getCell()->getBits();
    if(bit == 1)
        return ;

    vector<FF*> FFs = newFF->getClusterFF();
    // Stable Marriage Problem (Gale-Shapley Algorithm)
    std::vector<std::vector<double>> cost(bit, std::vector<double>(bit, 0)); // ith FF cost for putting it in j slot
    std::queue<int> waitSlot; // slot wait to be assigned FF
    std::priority_queue<std::pair<double, int>> pq[bit];
    for(int i=0;i<bit;i++){
        FF* curFF = FFs[i];
        for(int j=0;j<bit;j++){
            PrevInstance prevInstance = curFF->getPrevInstance();
            Coor newCoorD = newFF->getNewCoor() + newFF->getPinCoor("D" + std::to_string(j));
            double delta_hpwl = 0;
            Coor inputCoor;
            // D pin cost
            if(prevInstance.instance){
                if(prevInstance.cellType == CellType::IO){
                    inputCoor = prevInstance.instance->getCoor();
                    double old_hpwl = HPWL(inputCoor, curFF->getOriginalD());
                    double new_hpwl = HPWL(inputCoor, newCoorD);
                    delta_hpwl += old_hpwl - new_hpwl;
                }
                else if(prevInstance.cellType == CellType::GATE){
                    inputCoor = prevInstance.instance->getCoor() + prevInstance.instance->getPinCoor(prevInstance.pinName);
                    double old_hpwl = HPWL(inputCoor, curFF->getOriginalD());
                    double new_hpwl = HPWL(inputCoor, newCoorD);
                    delta_hpwl += old_hpwl - new_hpwl;
                }
                else{
                    FF* inputFF = dynamic_cast<FF*>(prevInstance.instance);
                    inputCoor = inputFF->getOriginalQ();
                    Coor newCoorQ = inputFF->getPhysicalFF()->getNewCoor() + inputFF->getPhysicalFF()->getPinCoor("Q" + inputFF->getPhysicalPinName());
                    double old_hpwl = HPWL(inputCoor, curFF->getOriginalD());
                    double new_hpwl = HPWL(newCoorQ, newCoorD);
                    delta_hpwl += old_hpwl - new_hpwl;
                }
            }
            double newSlack = curFF->getTimingSlack("D") + DisplacementDelay * delta_hpwl;
            cost[i][j] = newSlack < 0 ? newSlack : 0;

            // Q pin cost
            for(auto& nextFF : curFF->getNextStage()){
                Coor originalInput = curFF->getOriginalQ();
                Coor newInput = newFF->getNewCoor() + newFF->getPinCoor("Q" + std::to_string(j));
                if(nextFF.outputGate){
                    inputCoor = nextFF.outputGate->getCoor() + nextFF.outputGate->getPinCoor(nextFF.pinName);
                    double old_hpwl = HPWL(inputCoor, originalInput);
                    double new_hpwl = HPWL(inputCoor, newInput);
                    delta_hpwl = old_hpwl - new_hpwl;
                }
                else{
                    Coor newCoorD;
                    newCoorD = nextFF.ff->getPhysicalFF()->getNewCoor() + nextFF.ff->getPhysicalFF()->getPinCoor("D" + nextFF.ff->getPhysicalPinName());
                    double old_hpwl = HPWL(nextFF.ff->getOriginalD(), originalInput);
                    double new_hpwl = HPWL(newCoorD, newInput);
                    delta_hpwl = old_hpwl - new_hpwl;
                }
                newSlack = nextFF.ff->getTimingSlack("D") + DisplacementDelay * delta_hpwl;
                cost[i][j] += newSlack < 0 ? newSlack : 0;
            }
            pq[j].push({cost[i][j], i});
        }
        waitSlot.push(i);
    }

    for(int i=0;i<bit;i++)
        FFs[i]->setPhysicalFF(nullptr, -1);

    while(!waitSlot.empty()){
        int slot = waitSlot.front();
        int preferFF = pq[slot].top().second;   // this is idx = =
        pq[slot].pop();
        FF* curFF = FFs[preferFF];
        if(curFF->getPhysicalFF() != newFF){ // is a single FF
            curFF->setPhysicalFF(newFF, slot);
            newFF->addClusterFF(curFF, slot);
            waitSlot.pop();
        }
        else if(cost[preferFF][slot] < cost[preferFF][curFF->getSlot()]){ // is married, you want to be small three !!!!
            // the one who is not loved is small three ~~~~~
            waitSlot.push(curFF->getSlot()); // get find your new FF
            curFF->setPhysicalFF(newFF, slot);
            newFF->addClusterFF(curFF, slot);
            waitSlot.pop();
        }
    }
}

/**
 * @brief Debank the MBFF into single bit ff 
 * 
 * @param MBFF The merged MBFF
 * @param debankCellType The single bit FF celltype to debank
 * @return std::vector<FF*> The vector contains all signle bit FF
 * @todo the incremental debanking
 */
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
        newFF->setClusterSize(1);
        newFF->addClusterFF(ff, 0);
        newFF->setClkIdx(clkIdx);
        newFF->setFixed(false);
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

/**
 * @brief get TNS
 * 
 * @return double return the total negative slack value (+: has negative slack, 0: no negative slack)
 */
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

/**
 * @brief the cost function without evaluate the bin density
 * 
 * @param verbose whether to print out the pretty table
 * @return double overall cost
 */
double Manager::getOverallCost(bool verbose){
    double TNS_cost = 0;
    double Power_cost = 0;
    double Area_cost = 0;
    double Bin_cost = 0;
    for(const auto & ff_pair : FF_Map){
        double curTNS = ff_pair.second->getTNS();
        TNS_cost += alpha * (curTNS);
        Power_cost += beta * ff_pair.second->getCell()->getGatePower();
        Area_cost += gamma * (ff_pair.second->getCell()->getArea());
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

    double cost = TNS_cost + Power_cost + Area_cost + Bin_cost;
    double TNS_percentage = TNS_cost / cost * 100;
    double Power_percentage = Power_cost / cost * 100;
    double Area_percentage = Area_cost / cost * 100;
    double Bin_percentage = Bin_cost / cost * 100;
    if(verbose){
        if(SKIP_BIN_CALC) std::cout << "[Warning] Skip bin density calculation!" << std::endl;
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

/**
 * @brief score each cell based on ?????
 * @author cheng119 help confirm this score
 * What is this?? alpha*cell_vector[i]->getQpinDelay() * (cell_vector[i]->getW() + cell_vector[i]->getH())
 * does (cell_vector[i]->getW() + cell_vector[i]->getH()) to approximate HPWL???
 */
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

/**
 * @brief Predict the cost of the banked MBFF at newbankCoor
 * 
 * @param newbankCoor merge MBFF coordinate
 * @param bankCellType merge MBFF type
 * @param FFToBank All of FFs to merge
 * @return double (new cost - original cost)
 * 
 */
double Manager::getCostDiff(Coor newbankCoor, Cell* bankCellType, std::vector<FF*>& FFToBank){
    double costHPWL = 0;
    double costPower = 0;
    double costArea = 0;
    double costQ = 0;
    
    // bank to a psudo MBFF
    // size_t bit = bankCellType->getBits();
    // FF* newFF = getNewFF();
    // newFF->setCoor(newbankCoor);
    // newFF->setNewCoor(newbankCoor);
    // newFF->setCell(bankCellType);
    // newFF->setClusterSize(bit);
    // for(size_t i=0;i<FFToBank.size();i++){
    //     for(size_t j=0;j<FFToBank[i]->getClusterFF().size();j++){
    //         newFF->addClusterFF(FFToBank[i]->getClusterFF()[j], i + j);
    //         FFToBank[i]->getClusterFF()[j]->setPhysicalFF(newFF, i + j);
    //     }
    // }
    // assignSlot(newFF);

    // for(auto& ff : newFF->getClusterFF()){
    //     costHPWL += -(ff->getSlack());
    //     for(auto& next : ff->getNextStage())
    //         costHPWL += -(next.ff->getSlack());
    // }

    // for(size_t i=0;i<FFToBank.size();i++){
    //     for(size_t j=0;j<FFToBank[i]->getClusterFF().size();j++){
    //         FFToBank[i]->getClusterFF()[j]->setPhysicalFF(FFToBank[i], j);
    //     }
    // }
    // deleteFF(newFF);

    for(auto& MBFF : FFToBank){
        for(auto& ff : MBFF->getClusterFF()){ // old slack
            costHPWL -= -(ff->getSlack()); // D pin slack
            for(auto& next : ff->getNextStage())
                costHPWL -= -(next.ff->getSlack()); // Q pin relate slack
        }

        Coor oldCoor = MBFF->getNewCoor();
        MBFF->setNewCoor(newbankCoor);
        double QDiff = bankCellType->getQpinDelay() - MBFF->getCell()->getQpinDelay();

        for(auto& ff : MBFF->getClusterFF()){
            costHPWL += -(ff->getSlack());
            for(auto& next : ff->getNextStage())
                costHPWL += -(next.ff->getSlack());
            costQ += QDiff * ff->getNextStage().size();
        }
        costPower -= MBFF->getCell()->getGatePower();
        costArea -= MBFF->getCell()->getArea();
        MBFF->setNewCoor(oldCoor);
    }
    costPower += bankCellType->getGatePower();
    costArea += bankCellType->getArea();
    // costHPWL *= DisplacementDelay;
    return alpha * (costHPWL + costQ) + beta * costPower + gamma * costArea;
}