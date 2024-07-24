#include "Preprocess.h"
#include "OptimalLocation.h"

Preprocess::Preprocess(Manager& mgr) : mgr(mgr){

}

Preprocess::~Preprocess(){

}

void Preprocess::run(){
    FF::DisplacementDelay = mgr.DisplacementDelay;
    Debank();
    Build_Circuit_Gragh();
    optimal_FF_location();
}

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
        const Cell& cur_cell = *cur_ff.getCell();
        for(int i=0;i<cur_ff.getPinCount();i++){
            std::string pinName = cur_cell.getPinName(i);
            if(pinName[0] == 'D'){ // Assume all ff D pin start with D
                FF* temp = new FF;
                Coor ff_coor;
                Cell* ff_cell;
                ff_coor = cur_ff.getPinCoor(pinName) + cur_ff.getCoor() - targetCell->getPinCoor("D");  
                ff_cell = targetCell;
                double slack = cur_ff.getTimingSlack(pinName);
                // set original coor for D port and Q port
                Coor d_coor = cur_ff.getCoor() + cur_cell.getPinCoor(pinName);
                std::string QpinName = pinName;
                QpinName[0] = 'Q';
                Coor q_coor = cur_ff.getCoor() + cur_cell.getPinCoor(QpinName); 

                int ff_clk = cur_ff.getClkIdx();
                
                temp->setInstanceName(mgr.getNewFFName("FF"));
                temp->setCoor(ff_coor);
                temp->setTimingSlack("D", slack);
                temp->setOriginalCoor(d_coor, q_coor);
                temp->setOriginalQpinDelay(cur_cell.getQpinDelay());
                temp->setClkIdx(ff_clk);
                temp->setCell(ff_cell);
                FF_list[temp->getInstanceName()] = temp;
                FF_list_Map[cur_name+'/'+pinName] = temp->getInstanceName();
            }
        }
    }

    std::cout << "Total number of FF : " << FF_list.size() << std::endl;
}

void Preprocess::Build_Circuit_Gragh(){
    for(auto& n_m : mgr.Net_Map){
        const Net& n = n_m.second;
        std::string driving_cell;
        std::string driving_pin;
        bool is_CLK = false; // ignore clock net (maybe can done in parser)
        bool has_driving_cell = false;
        findDrivingCell(n, driving_cell, driving_pin, is_CLK, has_driving_cell);
        
        if(is_CLK || !has_driving_cell)
            continue;
        // Given the net and its dring cell
        // connect driving cell's output vector
        // and its output's input vector
        connectNet(n, driving_cell, driving_pin);
    }
    vector<std::string> deleteFF;
    for(auto& ff : FF_list_Map){
        if(FF_list[ff.second]->getInputInstances().size() == 0)
            deleteFF.push_back(ff.first);
    }
    for(size_t i=0;i<deleteFF.size();i++){
        FF_list.erase(FF_list_Map[deleteFF[i]]);
        FF_list_Map.erase(deleteFF[i]);
    }
    // go study STA
    DelayPropagation();
}

void Preprocess::optimal_FF_location(){
    // create FF logic
    std::unordered_map<std::string, int> idx_map;
    std::vector<FF*> FFs(FF_list.size());
    size_t i=0;
    for(auto& FF_m : FF_list){
        FFs[i] = FF_m.second;
        i++;
    }

    preprocessObjFunction obj(mgr, FF_list, idx_map, FF_list.size(), FFs);
    const double kAlpha = 100;
    Gradient optimizer(mgr, FF_list, obj, kAlpha, idx_map, FF_list.size(), FFs);

    std::cout << "Slack statistic before Optimize" << std::endl;
    double prevTNS = getSlackStatistic(true);
    const double terminateThreshold = 0.001;
    for(i=0;i<=1000;i++){
        optimizer.Step(true);
        // CAL new slack
        updateSlack();
        // update original data
        for(auto& ff_m : FF_list){
            FF* cur_ff = ff_m.second;
            cur_ff->setOriginalCoor(cur_ff->getCoor() + cur_ff->getPinCoor("D"), cur_ff->getCoor() + cur_ff->getPinCoor("Q"));
            cur_ff->setOriginalQpinDelay(cur_ff->getCell()->getQpinDelay());
        }
        if(i % 25 == 0){
            std::cout << "phase 1 step : " << i << std::endl;
            std::cout << "Slack statistic after Optimize" << std::endl;
        }
        double newTNS = getSlackStatistic(i%25 == 0);
        if(abs(newTNS - prevTNS) / abs(prevTNS) < terminateThreshold || newTNS == prevTNS){
            std::cout << "Gradient Convergen at " << i << " iteration." << std::endl;
            std::cout << "Final statistic" << std::endl;
            getSlackStatistic(true);
            break;
        }
        prevTNS = newTNS;
    }
}

//---------------------------------------------
//---------------------------------------------
// utils
//---------------------------------------------
//---------------------------------------------
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
        if(pinName[0] == 'Q' || (!p.getIsIOPin() && pinName.substr(0,3) == "OUT") || pinName.substr(0,5) == "INPUT"){
            driving_cell = p.getInstanceName();
            driving_pin = p.getPinName();
            has_driving_cell = true;
            break;
        }
    }
}
        

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
                else{ // output pin
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

void Preprocess::DelayPropagation(){
    // delay propagation
    // start with IO
    std::queue<Instance*> q;
    cout << "num of Input : " << mgr.Input_Map.size() << endl;
    for(auto& io_m : mgr.Input_Map){
        Instance* input = &mgr.IO_Map[io_m.first];
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
    while(!q.empty()){
        Instance* curInst = q.front();
        q.pop();

        if(mgr.Gate_Map.count(curInst->getInstanceName())){
            propagaGate(q, mgr.Gate_Map[curInst->getInstanceName()]);
        }
        else{
            assert(0 && "Something wrong!!!");
            // should never go here
            // its OUTPUT PIN
        }
    }
}

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
    double WNS = DBL_MAX;
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

    if(show){
        std::cout << "\tWorst negative slack : " << std::abs(WNS) << std::endl;
        std::cout << "\tTotal negative slack : " << std::abs(TNS) << std::endl;
        std::cout << "\tAverage slack : " << AVS / FF_list.size() << std::endl;
        std::cout << "\tMaximum slack : " << MAS << std::endl;
    }
    return TNS;
}

void Preprocess::updateSlack(){
    for(auto& ff_m : FF_list){
        FF* cur_ff = ff_m.second;
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
    } 
}