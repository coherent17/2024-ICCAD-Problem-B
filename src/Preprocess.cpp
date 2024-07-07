#include "Preprocess.h"
#include "OptimalLocation.h"

Preprocess::Preprocess(Manager& mgr) : mgr(mgr){

}

Preprocess::~Preprocess(){

}

void Preprocess::run(){
    Debank();
    Build_Circuit_Gragh();
    optimal_FF_location();
}

// Preprocess function
void Preprocess::Debank(){

    // select cell use after debank
    // if alread single bit it will use same cell as input given
    // map each FF to a single bit cell
    Cell* targetCell = nullptr;
    for(auto& cell_m : mgr.cell_library.getCellMap()){
        if(mgr.cell_library.isFF(cell_m.first) && cell_m.second->getBits() == 1){
            if(!targetCell)
                targetCell = cell_m.second;
            else if(targetCell->getW() > cell_m.second->getW())
                targetCell = cell_m.second; 
        }
    }

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
                const Cell* ff_cell;
                if(cur_cell.getBits() == 1){
                    ff_coor = cur_ff.getCoor();
                    ff_cell = cur_ff.getCell();
                }
                else{
                    ff_coor = cur_ff.getPinCoor(pinName) + cur_ff.getCoor();
                    ff_cell = targetCell;
                }
                double slack = cur_ff.getTimingSlack(pinName);
                // set original coor for D port and Q port
                Coor d_coor = cur_ff.getCoor() + cur_cell.getPinCoor(pinName);
                std::string QpinName = pinName;
                QpinName[0] = 'Q';
                Coor q_coor = cur_ff.getCoor() + cur_cell.getPinCoor(QpinName); 
                
                temp->setInstanceName(mgr.getNewFFName("FF"));
                temp->setCoor(ff_coor);
                temp->setTimingSlack("D", slack);
                temp->setOriginalCoor(d_coor, q_coor);
                temp->setOriginalQpinDelay(cur_cell.getQpinDelay());
                temp->setCell(ff_cell);
                temp->setCellName(ff_cell->getCellName());
                FF_list[temp->getInstanceName()] = temp;
                FF_list_Map[cur_name+'/'+pinName] = temp->getInstanceName();
            }
        }
    }

    std::cout << "Total number of FF : " << FF_list.size() << std::endl;

    // /* for debug
    // for(auto& lm : FF_list_Map){
    //     FF temp = *FF_list[lm.second];
    //     std::cout << lm.first << " map to logic ff " << temp.getInstanceName() << std::endl;
    //     std::cout << "With timing slacke : " << temp.getTimingSlack("D0") << " And Coor : " << temp.getCoor() << std::endl;
    //     std::cout << "Original D&Q coordinate : (" << temp.getOriginalD() << ", " << temp.getOriginalQ() << ")" << std::endl;
    //     std::cout << std::endl;
    // }
    // */
}

void Preprocess::Build_Circuit_Gragh(){
    for(auto& n_m : mgr.Net_Map){
        const Net& n = n_m.second;
        std::string driving_cell;
        std::string driving_pin;
        bool is_CLK = false; // ignore clock net (maybe can done in parser)
        bool has_driving_cell = false;
        // if(n.getNumPins() == 1) // ignore single pin nets
        //     continue;
        findDrivingCell(n, driving_cell, driving_pin, is_CLK, has_driving_cell);
        
        if(is_CLK || !has_driving_cell)
            continue;
        // Given the net and its dring cell
        // connect driving cell's output vector
        // and its output's input vector
        connectNet(n, driving_cell, driving_pin);
    }

    // find largest HPWL output instance
    // find out largest HPWL cell for IO and FF
    // (cur_FF->Q -----> Instance->inputPin)
    for(auto& ff_m : FF_list){
        setLargestOutput(ff_m.second);
    }

    for(auto& IO_m : mgr.IO_Map){
        setLargestOutput(&IO_m.second);
    }

    // find the largest HPWL input
    // trace the input FF for all FFs in FF list
    // (LargestInput->Q -----> comb -----> cur_FF->D)
    for(auto& ff_m : FF_list){
        setLargestInput(ff_m.second);
    }

    // find the next stage FF(cur_ff->Q ----> comb ----> nextFF->D)
    for(auto& ff_m : FF_list){
        if(ff_m.second->getLargestInput()){
            if(FF_list.count(ff_m.second->getLargestInput()->getInstanceName())){
                FF* input = FF_list[ff_m.second->getLargestInput()->getInstanceName()];
                input->setNextStageFF(ff_m.second);
            }
        }
    }
    // for debug
    // for(auto& lm : FF_list_Map){
    //     FF& temp = *FF_list[lm.second];
    //     std::cout << lm.first << " map to logic ff " << temp.getInstanceName() << std::endl;
    //     if(temp.getLargestInput() != nullptr)
    //         std::cout << "Input largest cell : " << temp.getLargestInput()->getInstanceName() << std::endl;
    //     else
    //         std::cout << "Input is floating" << std::endl << std::endl;
    //     if(temp.getLargestOutput().first != nullptr)
    //         std::cout << "Output largest cell : " << temp.getLargestOutput().first->getInstanceName() << std::endl << std::endl; 
    //     else
    //         std::cout << "Output is floating" << std::endl << std::endl;
    // }
    /*
    for(auto& lm : FF_list_Map){
        FF temp = FF_list[lm.second];
        cout << lm.first << " map to logic ff " << temp.getInstanceName() << endl;
        cout << "Its input from " << temp.getInput()[0] << endl;
        if(temp.getOutput().size())
            for(size_t i=0;i<temp.getOutput().size();i++)
                cout << "Its output to " << temp.getOutput()[i] << endl;
        else
            cout << "Its output is floating" << endl;
    }
    */
}

void Preprocess::optimal_FF_location(){
       // create FF logic
    std::vector<Coor> c(FF_list.size());
    std::unordered_map<std::string, int> idx_map;
    int i=0;
    for(auto&FF_m : FF_list){ // initial location and set index
        idx_map[FF_m.second->getInstanceName()] = i;
        c[i] = FF_m.second->getCoor();
        i++;
    }

    obj_function obj(mgr, FF_list, idx_map);
    const double kAlpha = 100;
    Gradient optimizer(mgr, FF_list, obj, c, kAlpha, idx_map);

    std::cout << "Slack statistic before Optimize" << std::endl;
    showSlackStatistic();

    for(i=0;i<=100;i++){
        std::cout << "phase 1 step : " << i << std::endl;
        optimizer.Step(true);
        // CAL new slack
        updateSlack();
        // update original data
        for(auto& ff_m : FF_list){
            FF* cur_ff = ff_m.second;
            cur_ff->setOriginalCoor(cur_ff->getCoor() + cur_ff->getPinCoor("D"), cur_ff->getCoor() + cur_ff->getPinCoor("Q"));
            cur_ff->setOriginalQpinDelay(cur_ff->getCell()->getQpinDelay());
        }

        std::cout << "Slack statistic after Optimize" << std::endl;
        showSlackStatistic();
        // if(i%50 == 0){
        //     mgr.FF_Map = FF_list;
        //     mgr.dumpVisual("after_" + std::to_string(i) + "_gradient");
        // }
    }
}

//---------------------------------------------
//---------------------------------------------
//---------------------------------------------
//---------------------------------------------
//---------------------------------------------
//---------------------------------------------
// utils
//---------------------------------------------
//---------------------------------------------
//---------------------------------------------
//---------------------------------------------
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
        if(instanceName == driving_cell)
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
                    FF_list[FF_list_Map[instanceName + '/' + p.getPinName()]]->addInput(driving_ff, driving_pin);
                    FF_list[driving_ff]->addOutput(FF_list[FF_list_Map[instanceName + '/' + p.getPinName()]]->getInstanceName(), pinName);
                }
                else if(mgr.Gate_Map.count(instanceName)){ // to std cell
                    mgr.Gate_Map[instanceName]->addInput(driving_ff, driving_pin);
                    FF_list[driving_ff]->addOutput(instanceName, pinName);
                }
                else{ // output pin
                    FF_list[driving_ff]->addOutput(instanceName, pinName);
                }
            }
            else{ // drive by std cell or IO
                if(mgr.FF_Map.count(instanceName)){ // to FF
                    FF_list[FF_list_Map[instanceName + '/' + p.getPinName()]]->addInput(driving_cell, driving_pin);
                    if(mgr.IO_Map.count(driving_cell))
                        mgr.IO_Map[driving_cell].addOutput(FF_list[FF_list_Map[instanceName + '/' + p.getPinName()]]->getInstanceName(), pinName);
                }
                else if(mgr.Gate_Map.count(instanceName)){ // to std cell
                    mgr.Gate_Map[instanceName]->addInput(driving_cell, driving_pin);
                    if(mgr.IO_Map.count(driving_cell))
                        mgr.IO_Map[driving_cell].addOutput(instanceName, pinName);
                }
                else{ // output pin
                    if(mgr.IO_Map.count(driving_cell))
                        mgr.IO_Map[driving_cell].addOutput(instanceName, pinName);
                }
            }
        }
    }
}

void Preprocess::setLargestOutput(Instance* driving_instance){
    // from all its output instances
    // findout one with largest HPWL as the largest output(critical path)
    Coor cur_coor;
    if(FF_list.count(driving_instance->getInstanceName())) // driving by FF
        cur_coor = FF_list[driving_instance->getInstanceName()]->getOriginalQ();
    else // driving by IO
        cur_coor = driving_instance->getCoor(); 
    double maxHPWL = 0;
    const std::vector<std::pair<std::string, std::string>>& output_vector = driving_instance->getOutputInstances();
    for(auto& instance_pair : output_vector){
        const std::string& instance_name = instance_pair.first;
        const std::string& pin_name = instance_pair.second;
        Instance *cur_instance;
        double curHPWL;
        if(FF_list.count(instance_name)){
            cur_instance = FF_list[instance_name];
            curHPWL = HPWL(cur_coor, FF_list[instance_name]->getOriginalD()); // in FF list coor is default as D pin coor
        }
        else if(mgr.Gate_Map.count(instance_name)){
            cur_instance = mgr.Gate_Map[instance_name];
            curHPWL = HPWL(cur_coor, cur_instance->getCoor() + cur_instance->getPinCoor(pin_name));
        }
        else{
            cur_instance = &mgr.IO_Map[instance_name];
            curHPWL = HPWL(cur_coor, cur_instance->getCoor());
        }
        if(curHPWL > maxHPWL){
            maxHPWL = curHPWL;
            driving_instance->setLargestOutput(cur_instance, pin_name);
        }
    }
}

void Preprocess::setLargestInput(FF* ff){
    std::vector<Instance*> temp_instance;
    std::queue<std::string> q;
    assert(ff->getInputInstances().size() != 0 && "FF with input floating");
    assert(ff->getInputInstances().size() == 1 && "FF should have only one driving cell");
    q.push(ff->getInputInstances()[0].first);
    while(!q.empty()){ // find out all input IO or FF, and save to temp_instance
        std::string cur_instance = q.front();
        q.pop();
        if(FF_list.count(cur_instance)){
            temp_instance.push_back(FF_list[cur_instance]);
        }
        else if(mgr.IO_Map.count(cur_instance)){
            temp_instance.push_back(&mgr.IO_Map[cur_instance]);
        }
        else{ // is std cell
            Gate& cur_gate = *mgr.Gate_Map[cur_instance];
            const std::vector<std::pair<std::string, std::string>>& input_vector = cur_gate.getInputInstances();
            for(size_t i=0;i<input_vector.size();i++)
                q.push(input_vector[i].first);
        }
    }

    double maxHPWL = 0;
    // find the one instance with largest output HPWL
    // max idx of HPWL(temp_instance, temp_instance largestOutput);
    for(auto& instance : temp_instance){
        bool isFF = mgr.FF_Map.count(instance->getInstanceName());
        double curHPWL;
        std::pair<Instance*, std::string> outputPair = instance->getLargestOutput();
        std::string outputInstanceName = outputPair.first->getInstanceName();
        Coor outputCellCoor;
        if(FF_list.count(outputInstanceName)){ // output to a FF
            outputCellCoor = FF_list[outputInstanceName]->getOriginalD();
        }
        else if(mgr.Gate_Map.count(outputInstanceName)){ // output to std cell
            outputCellCoor = outputPair.first->getCoor() + outputPair.first->getPinCoor(outputPair.second);
        }
        else{ // output to IO
            outputCellCoor = outputPair.first->getCoor();
        }
        if(isFF){
            curHPWL = HPWL(FF_list[FF_list_Map[instance->getInstanceName()]]->getOriginalQ(), outputCellCoor);
            curHPWL = FF_list[FF_list_Map[instance->getInstanceName()]]->getOriginalQpinDelay() + mgr.DisplacementDelay * curHPWL;
            if(curHPWL > maxHPWL){
                ff->setLargestInput(FF_list[FF_list_Map[instance->getInstanceName()]]);
                maxHPWL = curHPWL;
            }
        }
        else{ // is IO
            curHPWL = HPWL(instance->getCoor(), outputCellCoor);
            if(curHPWL > maxHPWL){
                ff->setLargestInput(instance);
                maxHPWL = curHPWL;
            }
        }
    }
}

void Preprocess::showSlackStatistic(){
        double WNS = DBL_MAX;
        double TNS = 0;
        double AVS = 0; // average slack
        double MAS = -DBL_MAX; // max slack
        //
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


        std::cout << "\tWorst negative slack : " << WNS << std::endl;
        std::cout << "\tTotal negative slack : " << TNS << std::endl;
        std::cout << "\tAverage slack : " << AVS / FF_list.size() << std::endl;
        std::cout << "\tMaximum slack : " << MAS << std::endl;
}

void Preprocess::updateSlack(){
    for(auto& ff_m : FF_list){
        FF* cur_ff = ff_m.second;

        // update slack for new location
        double delta_hpwl = 0;
        double delta_q = 0; // delta q pin delay
        // D pin delta HPWL
        std::string inputInstanceName = cur_ff->getInputInstances()[0].first;
        std::string inputPinName = cur_ff->getInputInstances()[0].second;
        Coor inputCoor;
        if(mgr.IO_Map.count(inputInstanceName)){
            inputCoor = mgr.IO_Map[inputInstanceName].getCoor();
            double old_hpwl = HPWL(inputCoor, cur_ff->getOriginalD());
            double new_hpwl = HPWL(inputCoor, cur_ff->getCoor() + cur_ff->getPinCoor("D"));
            delta_hpwl += old_hpwl - new_hpwl;
        }
        else if(mgr.Gate_Map.count(inputInstanceName)){
            inputCoor = mgr.Gate_Map[inputInstanceName]->getCoor() + mgr.Gate_Map[inputInstanceName]->getPinCoor(inputPinName);
            double old_hpwl = HPWL(inputCoor, cur_ff->getOriginalD());
            double new_hpwl = HPWL(inputCoor, cur_ff->getCoor() + cur_ff->getPinCoor("D"));
            delta_hpwl += old_hpwl - new_hpwl;
        }
        else{
            inputCoor = FF_list[inputInstanceName]->getOriginalQ();
            double old_hpwl = HPWL(inputCoor, cur_ff->getOriginalD());
            double new_hpwl = HPWL(FF_list[inputInstanceName]->getCoor() + FF_list[inputInstanceName]->getPinCoor("Q"), cur_ff->getCoor() + cur_ff->getPinCoor("D"));
            delta_hpwl += old_hpwl - new_hpwl;
        }

        // Q pin delta HPWL (prev stage FFs Qpin)
        if(cur_ff->getLargestInput()){
            std::string prevInstanceName = cur_ff->getLargestInput()->getInstanceName();
            Coor originalInput, newInput;
            if(mgr.IO_Map.count(prevInstanceName)){
                originalInput = mgr.IO_Map[prevInstanceName].getCoor();
                newInput = originalInput;
            }
            else if(FF_list.count(prevInstanceName)){
                originalInput = FF_list[prevInstanceName]->getOriginalQ();
                newInput = FF_list[prevInstanceName]->getCoor() + FF_list[prevInstanceName]->getPinCoor("Q");
                delta_q = FF_list[prevInstanceName]->getOriginalQpinDelay() - FF_list[prevInstanceName]->getCell()->getQpinDelay();
            }

            std::pair<Instance*, std::string> output = cur_ff->getLargestInput()->getLargestOutput();
            if(output.first){
                std::string outputInstanceName = output.first->getInstanceName();
                if(mgr.IO_Map.count(outputInstanceName)){
                    inputCoor = mgr.IO_Map[outputInstanceName].getCoor();
                    double old_hpwl = HPWL(inputCoor, originalInput);
                    double new_hpwl = HPWL(inputCoor, newInput);
                    delta_hpwl += old_hpwl - new_hpwl;
                }
                else if(mgr.Gate_Map.count(outputInstanceName)){
                    inputCoor = mgr.Gate_Map[outputInstanceName]->getCoor() + mgr.Gate_Map[outputInstanceName]->getPinCoor(output.second);
                    double old_hpwl = HPWL(inputCoor, originalInput);
                    double new_hpwl = HPWL(inputCoor, newInput);
                    delta_hpwl += old_hpwl - new_hpwl;
                }
                else{
                    inputCoor = FF_list[outputInstanceName]->getOriginalD();
                    double old_hpwl = HPWL(inputCoor, originalInput);
                    double new_hpwl = HPWL(FF_list[outputInstanceName]->getCoor() + FF_list[outputInstanceName]->getPinCoor("D"), newInput);
                    delta_hpwl += old_hpwl - new_hpwl;
                }
            }
        }
        // get new slack
        double newSlack = cur_ff->getTimingSlack("D") + (delta_q) + mgr.DisplacementDelay * delta_hpwl;
        cur_ff->setTimingSlack("D", newSlack);
    } 
}