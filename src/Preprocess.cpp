#include "Preprocess.h"
#include "OptimalLocation.h"

Preprocess::Preprocess(Manager& mgr) : mgr(mgr), changed(false){
}

Preprocess::~Preprocess(){

}

/**
 * @brief 
 * @cheng119, integrate alpha, beta, gamma, lambda into paramMgr
 * what is changed stand for??
 */
void Preprocess::run(){
    FF::DisplacementDelay = mgr.DisplacementDelay;
    FF::alpha = mgr.alpha;
    FF::beta = mgr.beta;
    FF::gamma = mgr.gamma;
    changed = false;
    Debank();
    Build_Circuit_Gragh();
    ChangeCell();
    // if(changed)
    optimal_FF_location();
}

/**
 * @brief Debank all of the MBFF is exists in input, and assign the suitible celltype
 * 
 */
// Preprocess function
void Preprocess::Debank(){
    // select cell use after debank
    // use the FF with lowest cost
    Cell* targetCell = mgr.Bit_FF_Map[1][0];

    // debank and save all the FF in logic_FF;
    // which is all one bit ff without technology mapping(no cell library)
    for(auto& mbff_m : mgr.FF_Map){
        FF& cur_ff = *mbff_m.second;
        const std::string& cur_name = mbff_m.first;
        Cell* cur_cell = cur_ff.getCell();
        for(int i=0;i<cur_ff.getPinCount();i++){
            std::string pinName = cur_cell->getPinName(i);
            if(pinName[0] == 'D'){ // Assume all ff D pin start with D
                FF* temp = new FF;
                Coor ff_coor;
                Cell* ff_cell;
                if(cur_cell->getBits() != 1){
                    // find the debanking single bit cell bottom-left coordinate
                    ff_coor = cur_ff.getPinCoor(pinName) + cur_ff.getCoor() - targetCell->getPinCoor("D");  
                    ff_cell = targetCell;
                    changed = true;
                    temp->setFixed(false);
                }
                else{
                    ff_coor = cur_ff.getCoor();
                    ff_cell = cur_cell;
                }
                double slack = cur_ff.getTimingSlack(pinName);
                // set original coor for D port and Q port
                Coor d_coor = cur_ff.getCoor() + cur_cell->getPinCoor(pinName);
                std::string QpinName = pinName;
                QpinName[0] = 'Q';
                Coor q_coor = cur_ff.getCoor() + cur_cell->getPinCoor(QpinName); 

                int ff_clk = cur_ff.getClkIdx();
                
                temp->setInstanceName(mgr.getNewFFName("FF"));
                temp->setCoor(ff_coor);
                temp->setNewCoor(ff_coor);
                temp->setTimingSlack("D", slack);
                temp->setOriginalCoor(d_coor, q_coor);
                temp->setOriginalQpinDelay(cur_cell->getQpinDelay());
                temp->setClkIdx(ff_clk);
                temp->setCell(ff_cell);

                // map all of the single bit name to its pointer
                FF_list[temp->getInstanceName()] = temp;
                FF_list_Map[cur_name+'/'+pinName] = temp->getInstanceName();
            }
        }
    }
    #ifndef NDEBUG
    std::cout << "Total number of FF : " << FF_list.size() << std::endl;
    #endif
}

/**
 * @brief Main core function to build the graph data structure to traverse
 * 
 */
void Preprocess::Build_Circuit_Gragh(){
    vector<Net*> NetList(mgr.Net_Map.size());
    size_t NetIDX = 0;
    for(auto& n_m : mgr.Net_Map){
        NetList[NetIDX] = &n_m.second;
        NetIDX++;
    }

    for(size_t i=0;i<NetIDX;i++){
        const Net& n = *NetList[i];
        std::string driving_cell;
        std::string driving_pin;
        bool is_CLK = false; // ignore clock net (maybe can done in parser)
        bool has_driving_cell = false;
        findDrivingCell(n, driving_cell, driving_pin, is_CLK, has_driving_cell);
        
        if(is_CLK || !has_driving_cell)
            continue;
        // Given the net and its drivng cell
        // connect driving cell's output vector
        // and its output's input vector
        connectNet(n, driving_cell, driving_pin);
    }

    // 08/21 : delete as input design will not contain floating FF
    // deal with the open circuit??
    // vector<std::string> deleteFF;
    // for(auto& ff : FF_list_Map){
    //     if(FF_list[ff.second]->getInputInstances().size() == 0 && FF_list[ff.second]->getOutputInstances().size() == 0)
    //         deleteFF.push_back(ff.first);
    // }
    // for(size_t i=0;i<deleteFF.size();i++){
    //     FF_list.erase(FF_list_Map[deleteFF[i]]);
    //     FF_list_Map.erase(deleteFF[i]);
    // }
    // go study STA
    DelayPropagation();
}

/**
 * @brief Main function to use CG to shift the ff to the optimal location
 * 
 */
void Preprocess::optimal_FF_location(){
    double prevTNS = getSlackStatistic(false);
    // create FF logic
    std::unordered_map<std::string, int> idx_map;
    std::vector<FF*> FFsFixed;
    std::vector<FF*> FFs(FF_list.size());
    FFsFixed.reserve(FF_list.size());
    size_t i=0;
    for(auto& FF_m : FF_list){
        FF* curFF = FF_m.second;
        if(1 || !curFF->getFixed()){ // ignore fixed idea, it does improve :( 
            FFsFixed.push_back(curFF);
        }
        FFs[i] = curFF;
        i++;
    }
    preprocessObjFunction obj(mgr, FF_list, idx_map, FFsFixed.size(), FFsFixed);
    const double kAlpha = mgr.Bit_FF_Map[1][0]->getW() / std::max(mgr.alpha, 10.0);
    Gradient optimizer(mgr, FF_list, obj, kAlpha, idx_map, FFsFixed.size(), FFsFixed);

    double terminateThreshold = 0.001;
    for(i=0;i<=1000;i++){
        optimizer.Step(true);
        // CAL new slack
        double TNS = updateSlack(FFs);
        // update original data
        for(auto& ff_m : FF_list){
            FF* cur_ff = ff_m.second;
            cur_ff->setOriginalCoor(cur_ff->getCoor() + cur_ff->getPinCoor("D"), cur_ff->getCoor() + cur_ff->getPinCoor("Q"));
        }
        if(i % 25 == 0){
            #ifndef NDEBUG
            std::cout << "phase 1 step : " << i << std::endl;
            std::cout << "TNS : " << TNS << std::endl;
            #endif
        }
        double newTNS = TNS;
        if(newTNS > prevTNS)
            terminateThreshold *= 1.1;
        if(abs(newTNS - prevTNS) / abs(prevTNS) < terminateThreshold || newTNS == prevTNS ){
            #ifndef NDEBUG
            std::cout << "Gradient Convergen at " << i << " iteration." << std::endl;
            std::cout << "Final statistic" << std::endl;
            #endif
            getSlackStatistic(true);
            break;
        }
        prevTNS = newTNS;
    }
}

/**
 * @brief Do technology mapping for all debankded single bit ff
 * @attention  Does forcesmaller to help to choose more-legalizable celltype, since you use 
 * "targetCell->getW() <= curCell->getW() && targetCell->getH() <= curCell->getH()" as constraint
 */
void Preprocess::ChangeCell(){
    size_t bitMapSize = mgr.Bit_FF_Map[1].size();
    vector<FF*> FFs(FF_list.size());
    size_t FFidx = 0;
    for(auto& ff_m : FF_list){
        FFs[FFidx] = ff_m.second;
        FFidx++;
    }
    // bool forceSmaller = mgr.alpha / (mgr.alpha + mgr.beta + mgr.gamma) > 0.1;
    // debank and save all the FF in logic_FF;
    // which is all one bit ff without technology mapping(no cell library)
    for(size_t i=0;i<FFs.size();i++){
        FF* curFF = FFs[i];
        double bestCost = 0;
        Cell* bestCell = curFF->getCell();

        // iterate through all single bit cell type
        for(size_t j=0;j<bitMapSize;j++){
            Cell* targetCell = mgr.Bit_FF_Map[1][j];
            double TimingCost = 0;
            Cell* curCell = curFF->getCell();
            // delta q pin delay will propagate to all fanout endpoints
            TimingCost += (targetCell->getQpinDelay() - curCell->getQpinDelay()) * curFF->getNextStage().size();
            TimingCost += mgr.DisplacementDelay * (
                    HPWL(curFF->getCoor() + curFF->getPinCoor("D"), curFF->getCoor() + targetCell->getPinCoor("D"))
                +   HPWL(curFF->getCoor() + curFF->getPinCoor("Q"), curFF->getCoor() + targetCell->getPinCoor("Q"))  * curFF->getNextStage().size()           
                                );
            double AreaCost = targetCell->getArea() - curCell->getArea();
            double PowerCost = targetCell->getGatePower() - curCell->getGatePower();
            double totalCost = mgr.alpha * TimingCost + mgr.beta * PowerCost + mgr.gamma * AreaCost;
            // hard constraint for using smaller cell, for easier legalize, need reconsider
            if(totalCost < bestCost){// && ((targetCell->getW() <= curCell->getW() && targetCell->getH() <= curCell->getH()) || !forceSmaller)){
                bestCost = totalCost;
                bestCell = targetCell;
            }
        }
        if(bestCost != 0){
            changed = true;
            curFF->setCell(bestCell);
            curFF->setFixed(false);
        }
    }

    // update slack and change origialQpinDelay
    updateSlack(FFs);
    for(size_t i=0;i<FFs.size();i++){
        FF* curFF = FFs[i];
        curFF->setOriginalQpinDelay(curFF->getCell()->getQpinDelay());
    }
}

//---------------------------------------------
//---------------------------------------------
// utils
//---------------------------------------------
//---------------------------------------------

/**
 * @brief find the driving cell of the net
 * 
 * @param n (net): the net to find the driving cell
 * @param driving_cell 
 * @param driving_pin 
 * @param is_CLK 
 * @param has_driving_cell 
 * @attention 1. Is there any situation that not a clk net has multiple output from the previous stage?
 * @attention 2. Is there any situation that not a clk net has no driving cell?
 */
void Preprocess::findDrivingCell(const Net& n, std::string& driving_cell, std::string& driving_pin, 
                                 bool& is_CLK, bool& has_driving_cell){
    // Given the net, find its driving cell or state it is clk net
    for(int i=0;i<n.getNumPins();i++){
        // from testcase release it seems driving cell is first pin of net, but not confirm by ICCAD
        const Pin& p = n.getPin(i);
        const std::string& pinName = p.getPinName();
        if(pinName.substr(0, 3) == "CLK"){
            is_CLK = true;
            break;
        }
        
        if(pinName[0] == 'Q' || (!p.getIsIOPin() && pinName.substr(0,3) == "OUT")/*for gate output*/ || pinName.substr(0,5) == "INPUT"){
            driving_cell = p.getInstanceName();
            driving_pin = p.getPinName();
            has_driving_cell = true;
            break;
        }
    }
}
        
/**
 * @brief connect the cell or IO pin to the previous stage and the next stage
 * 
 * @param n (Net)
 * @param driving_cell 
 * @param driving_pin 
 */
void Preprocess::connectNet(const Net& n, std::string& driving_cell, std::string& driving_pin){
    // Given the net and its dring cell
    // connect driving cell's output vector
    // and its output's input vector
    for(int i=0;i<n.getNumPins();i++){ // build gragh
        const Pin& p = n.getPin(i);
        const std::string& instanceName = p.getInstanceName();
        const std::string& pinName = p.getPinName();
        if((instanceName == driving_cell) && (pinName == driving_pin))
            continue;
        else{
            if(p.getPinName().substr(0, 3) == "CLK"){
                break;
            }
            // set output cells for driving cell(only for FF)
            if(mgr.FF_Map.count(driving_cell)){ // drive by FF
                std::string driving_ff = driving_cell + "/D" + driving_pin.substr(1, driving_pin.size()-1);
                driving_ff = FF_list[FF_list_Map[driving_ff]]->getInstanceName();
                if(mgr.FF_Map.count(instanceName)){ // to FF
                    FF_list[FF_list_Map[instanceName + '/' + p.getPinName()]]->addInput("D", driving_ff, "Q");
                    FF_list[driving_ff]->addOutput("Q", FF_list[FF_list_Map[instanceName + '/' + p.getPinName()]]->getInstanceName(), pinName);
                }
                else if(mgr.Gate_Map.count(instanceName)){ // to std cell
                    mgr.Gate_Map[instanceName]->addInput(pinName, driving_ff, "Q");
                    FF_list[driving_ff]->addOutput("Q", instanceName, pinName);
                }
                else{ // output pin
                    FF_list[driving_ff]->addOutput("Q", instanceName, pinName);
                }
            }
            else{ // drive by std cell or IO
                if(mgr.FF_Map.count(instanceName)){ // to FF
                    FF_list[FF_list_Map[instanceName + '/' + p.getPinName()]]->addInput("D", driving_cell, driving_pin);
                    if(mgr.IO_Map.count(driving_cell))
                        mgr.IO_Map[driving_cell].addOutput(driving_pin, FF_list[FF_list_Map[instanceName + '/' + p.getPinName()]]->getInstanceName(), pinName);
                    else
                        mgr.Gate_Map[driving_cell]->addOutput(driving_pin, FF_list[FF_list_Map[instanceName + '/' + p.getPinName()]]->getInstanceName(), pinName);
                }
                else if(mgr.Gate_Map.count(instanceName)){ // to std cell
                    mgr.Gate_Map[instanceName]->addInput(pinName, driving_cell, driving_pin);
                    if(mgr.IO_Map.count(driving_cell))
                        mgr.IO_Map[driving_cell].addOutput(driving_pin, instanceName, pinName);
                    else
                        mgr.Gate_Map[driving_cell]->addOutput(driving_pin, instanceName, pinName);
                }
                else{ // to output pin
                    mgr.IO_Map[instanceName].addInput(pinName, driving_cell, driving_pin);
                    if(mgr.IO_Map.count(driving_cell))
                        mgr.IO_Map[driving_cell].addOutput(driving_pin, instanceName, pinName);
                    else
                        mgr.Gate_Map[driving_cell]->addOutput(driving_pin, instanceName, pinName);
                }
            }
        }
    }
}


/**
 * @brief find out cost between reg2reg in topological order (implemented by queue), include
 *        1. qpin delay
 *        2. hpwl * displacement delay
 */
void Preprocess::DelayPropagation(){
    // delay propagation
    // start with IO
    std::queue<Instance*> q;
    // Iterate all of the input
    for(auto& io_m : mgr.Input_Map){
        Instance* input = &mgr.IO_Map[io_m.first];

        // Iterate all of the instance that input port connect with
        for(auto& outputPairs : input->getOutputInstances()){
            const std::string& drivingPin = outputPairs.first;
            for(auto& outputVector : outputPairs.second){
                const std::string& instanceName = outputVector.first;
                const std::string& pinName = outputVector.second;
                if(FF_list.count(instanceName)){// output to FF
                    FF_list[instanceName]->setPrevInstance({input, CellType::IO, drivingPin});
                }
                else if(mgr.Gate_Map.count(instanceName)){ // output to std cell
                    Gate* curGate = mgr.Gate_Map[instanceName];
                    double cost = mgr.DisplacementDelay * HPWL(input->getCoor(), curGate->getCoor() + curGate->getPinCoor(pinName));
                    curGate->setMaxInput({input, curGate, drivingPin, cost});
                    curGate->updateVisitedTime();
                    if(curGate->getVisitedTime() == curGate->getCell()->getInputCount()){
                        q.push(curGate);
                    }
                }
            }
        }
    }

    // propagate FF, its like INPUT
    for(auto& ff_m: FF_list){
        propagaFF(q, ff_m.second);
    }

    // propagation
    // std::unordered_map<std::string, bool> visitedGate;
    while(!q.empty()){
        Instance* curInst = q.front();
        q.pop();
        propagaGate(q, mgr.Gate_Map[curInst->getInstanceName()]);
    }

    // for(auto& gate : mgr.Gate_Map){
    //     if(!visitedGate.count(gate.first)){
    //         // std::cout << "[WARNING] Gate " + gate.first + " didn't visited!!!" << std::endl;
    //     }
    // }
}

/**
 * @brief set cost of FF to next stage(FF or standard cell) 
 * 
 * @param q (queue), used in topological sort, the order of the gate to traverse
 * @param ff start from ff, find the next stage cost
 */
void Preprocess::propagaFF(std::queue<Instance*>& q, FF* ff){
    // given the ff, setMaxInput for all its output std cell
    // and prevInstance for all its output FF
    const std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>>& outputMap = ff->getOutputInstances();
    for(const auto& output_m : outputMap){
        const std::string& outputPin = output_m.first;
        assert(outputPin == "Q" && "FF list ff output pin should always be Q");
        for(const auto& outputVector : output_m.second){
            const std::string& instanceName = outputVector.first;
            const std::string& pinName = outputVector.second;

            if(FF_list.count(instanceName)){// FF output to FF
                FF_list[instanceName]->setPrevInstance({ff, CellType::FF, "Q"});
                ff->addNextStage({FF_list[instanceName], nullptr, " "});
            }
            else if(mgr.Gate_Map.count(instanceName)){ // FF output to std cell
                Gate* curGate = mgr.Gate_Map[instanceName];
                double cost = ff->getOriginalQpinDelay() + mgr.DisplacementDelay * HPWL(ff->getOriginalQ(), curGate->getCoor() + curGate->getPinCoor(pinName));
                curGate->setMaxInput({ff, curGate, pinName, cost});
                curGate->updateVisitedTime();
                if(curGate->getVisitedTime() == curGate->getCell()->getInputCount()){
                    q.push(curGate);
                }
            }
        }
    }
}

/**
 * @brief set cost of gate to next stage(FF or standard cell) 
 * 
 * @param q (queue), used in topological sort, the order of the gate to traverse
 * @param gate start from gate, find the next stage cost
 */
void Preprocess::propagaGate(std::queue<Instance*>& q, Gate* gate){
    MaxInput maxInput = gate->getMaxInput();
    FF* prevFF = nullptr;
    if(FF_list.count(maxInput.instance->getInstanceName())){
        prevFF = FF_list[maxInput.instance->getInstanceName()];
    }
    const std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>>& outputMap = gate->getOutputInstances();
    for(const auto& output_m : outputMap){
        const std::string& outputPin = output_m.first;
        for(const auto& outputVector : output_m.second){
            const std::string& instanceName = outputVector.first;
            const std::string& pinName = outputVector.second;

            if(FF_list.count(instanceName)){// std cell output to FF
                FF_list[instanceName]->setPrevInstance({gate, CellType::GATE, outputPin});

                // set up for prev stage FF
                if(prevFF){ // ignore when start of critical path is INPUT
                    FF_list[instanceName]->setPrevStage({prevFF, maxInput.outputGate, maxInput.pinName});
                    prevFF->addNextStage({FF_list[instanceName], maxInput.outputGate, maxInput.pinName});
                }
            }
            else if(mgr.Gate_Map.count(instanceName)){ // std cell to std cell
                Gate* curGate = mgr.Gate_Map[instanceName];
                double cost = maxInput.cost + mgr.DisplacementDelay * 
                            HPWL(gate->getCoor() + gate->getPinCoor(outputPin), curGate->getCoor() + curGate->getPinCoor(pinName));
                curGate->setMaxInput({maxInput.instance, maxInput.outputGate, maxInput.pinName, cost}); // AUTOMATICALLY update with largest cost
                curGate->updateVisitedTime();
                if(curGate->getVisitedTime() == curGate->getCell()->getInputCount()){
                    q.push(curGate);
                }
            }
        }
    }
}

double Preprocess::getSlackStatistic(bool show){
    double WNS = 0;
    double TNS = 0;
    double AVS = 0; // average slack
    double MAS = -DBL_MAX; // max slack

    for(auto& ff_m : FF_list){
        FF* temp = ff_m.second;
        double slack = temp->getTimingSlack("D");
        if(slack < WNS)
            WNS = slack;
        if(slack < 0)
            TNS += slack;
        AVS += slack;
        if(slack > MAS)
            MAS = slack;
    }
    #ifndef NDEBUG
    if(show){
        std::cout << "\tWorst negative slack : " << std::abs(WNS) << std::endl;
        std::cout << "\tTotal negative slack : " << std::abs(TNS) << std::endl;
        std::cout << "\tAverage slack : " << AVS / FF_list.size() << std::endl;
        std::cout << "\tMaximum slack : " << MAS << std::endl;
    }
    #endif
    return TNS;
}

/**
 * @brief Calculating the hpwl diff from prev instance & prev stage, update the dslack
 * 
 * @param FFs a vector of ff to update slack
 * @return double the TNS (+: has negative slack, 0: no negative slack)
 * @formula SFFN’ = SFFN + (δ0 – δ0’) + DisplacementDelay * (WLQ0 - WLQ'0) + DisplacementDelay * (WLD_N - WLD'N)
 */
double Preprocess::updateSlack(std::vector<FF*>& FFs){\
    double TNS = 0;
    #pragma omp parallel for reduction(+:TNS)
    for(size_t i=0;i<FFs.size();i++){
    // for(auto& ff_m : FF_list){
        FF* cur_ff = FFs[i];
        // update slack for new location
        double delta_hpwl = 0;
        double delta_q = 0; // delta q pin delay
        Coor inputCoor;
        // D pin delta HPWL
        PrevInstance prevInstance = cur_ff->getPrevInstance();
        if(prevInstance.instance){
            if(prevInstance.cellType == CellType::IO){
                inputCoor = prevInstance.instance->getCoor();
                double old_hpwl = HPWL(inputCoor, cur_ff->getOriginalD());
                double new_hpwl = HPWL(inputCoor, cur_ff->getCoor() + cur_ff->getPinCoor("D"));
                delta_hpwl += old_hpwl - new_hpwl;
            }
            else if(prevInstance.cellType == CellType::GATE){
                inputCoor = prevInstance.instance->getCoor() + prevInstance.instance->getPinCoor(prevInstance.pinName);
                double old_hpwl = HPWL(inputCoor, cur_ff->getOriginalD());
                double new_hpwl = HPWL(inputCoor, cur_ff->getCoor() + cur_ff->getPinCoor("D"));
                delta_hpwl += old_hpwl - new_hpwl;
            }
            else{
                FF* prevFF = dynamic_cast<FF*>(prevInstance.instance);
                inputCoor = prevFF->getOriginalQ();
                double old_hpwl = HPWL(inputCoor, cur_ff->getOriginalD());
                double new_hpwl = HPWL(prevFF->getCoor() + prevFF->getPinCoor("Q"), cur_ff->getCoor() + cur_ff->getPinCoor("D"));
                delta_hpwl += old_hpwl - new_hpwl;
            }
        }

        // Q pin delta HPWL (prev stage FFs Qpin)
        const PrevStage& prev = cur_ff->getPrevStage();
        if(prev.ff){
            Coor originalInput, newInput;
            originalInput = prev.ff->getOriginalQ();
            newInput = prev.ff->getCoor() + prev.ff->getPinCoor("Q");
            delta_q = prev.ff->getOriginalQpinDelay() - prev.ff->getCell()->getQpinDelay();

            inputCoor = prev.outputGate->getCoor() + prev.outputGate->getPinCoor(prev.pinName);
            double old_hpwl = HPWL(inputCoor, originalInput);
            double new_hpwl = HPWL(inputCoor, newInput);
            delta_hpwl += old_hpwl - new_hpwl;
        }
        // get new slack
        double newSlack = cur_ff->getTimingSlack("D") + (delta_q) + mgr.DisplacementDelay * delta_hpwl;
        cur_ff->setTimingSlack("D", newSlack);
        if(newSlack < 0)
            TNS += std::abs(newSlack);
    } 
    return TNS;
}
