#include "Manager.h"
#include "OptimalLocation.h"
Manager::Manager():
    alpha(0),
    beta(0),
    gamma(0),
    lambda(0),
    DisplacementDelay(0),
    NumInput(0),
    NumOutput(0),
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
}

void Manager::parse(const std::string &filename){
    Parser parser(filename);
    parser.parse(*this);
}

void Manager::meanshift(){
    // do graceful meanshift clustering
    std::cout << "do graceful meanshift clustering..." << std::endl;
    MeanShift meanshift;
    meanshift.run(*this);
}

void Manager::dump(const std::string &filename){
    Dumper dumper(filename);
    dumper.dump(*this);
}

void Manager::dumpVisual(const std::string &filename){
    std::ofstream fout;
    fout.open(filename.c_str());
    assert(fout.good());

    fout << "Diesize " << die.getDieOrigin().x << " " << die.getDieOrigin().y << " " << die.getDieBorder().x << " " << die.getDieBorder().y << std::endl;

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

    // for(cell_library){
    //     fout << "FlipFlop " << pair.second->
    // }
    std::map<std::string, FF *> ff_map(FF_Map.begin(), FF_Map.end());
    for(const auto &pair: ff_map){
        fout << "Inst " << pair.first << " " << pair.second->getCellName() << " " << pair.second->getNewCoor().x << " " << pair.second->getNewCoor().y << std::endl;
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

void Manager::Debank(){
    // debank and save all the FF in logic_FF;
    // which is all one bit ff without technology mapping(no cell library)
    for(auto& mbff_m : FF_Map){
        FF& cur_ff = *mbff_m.second;
        const std::string& cur_name = mbff_m.first;
        const Cell& cur_cell = *cur_ff.getCell();
        for(int i=0;i<cur_ff.getPinCount();i++){
            std::string pinName = cur_cell.getPinName(i);
            if(pinName[0] == 'D'){ // Assume all ff D pin start with D
                FF* temp = new FF;
                Coor ff_coor = cur_ff.getPinCoor(pinName) + cur_ff.getCoor();
                double slack = cur_ff.getTimingSlack(pinName);
                temp->setInstanceName(getNewFFName("FF"));
                temp->setCoor(ff_coor);
                temp->setTimingSlack("D", slack);
                // set original coor for D port and Q port
                Coor d_coor = cur_ff.getCoor() + cur_cell.getPinCoor(pinName);
                std::string QpinName = pinName;
                QpinName[0] = 'Q';
                Coor q_coor = cur_ff.getCoor() + cur_cell.getPinCoor(QpinName); 
                temp->setOriginalCoor(d_coor, q_coor);
                temp->setOriginalQpinDelay(cur_cell.getQpinDelay());
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

void Manager::Build_Circuit_Gragh(){
    for(auto& n_m : Net_Map){
        const Net& n = n_m.second;
        std::string driving_cell;
        std::string driving_pin;
        bool is_CLK = false; // ignore clock net (maybe can done in parser)
        bool has_driving_cell = false;
        // if(n.getNumPins() == 1) // ignore single pin nets
        //     continue;
        for(int i=0;i<n.getNumPins();i++){ // find driving cell of this net
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
        if(is_CLK || !has_driving_cell)
            continue;

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
                if(FF_Map.count(driving_cell)){ // drive by FF
                    std::string driving_ff = driving_cell + "/D" + driving_pin.substr(1, driving_pin.size()-1);
                    driving_ff = FF_list[FF_list_Map[driving_ff]]->getInstanceName();
                    if(FF_Map.count(instanceName)){ // to FF
                        FF_list[FF_list_Map[instanceName + '/' + p.getPinName()]]->addInput(driving_ff, driving_pin);
                        FF_list[driving_ff]->addOutput(FF_list[FF_list_Map[instanceName + '/' + p.getPinName()]]->getInstanceName(), pinName);
                    }
                    else if(Gate_Map.count(instanceName)){ // to std cell
                        Gate_Map[instanceName]->addInput(driving_ff, driving_pin);
                        FF_list[driving_ff]->addOutput(instanceName, pinName);
                    }
                    else{ // output pin
                        FF_list[driving_ff]->addOutput(instanceName, pinName);
                    }
                }
                else{ // drive by std cell or IO
                    if(FF_Map.count(instanceName)){ // to FF
                        FF_list[FF_list_Map[instanceName + '/' + p.getPinName()]]->addInput(driving_cell, driving_pin);
                        if(IO_Map.count(driving_cell))
                            IO_Map[driving_cell].addOutput(FF_list[FF_list_Map[instanceName + '/' + p.getPinName()]]->getInstanceName(), pinName);
                    }
                    else if(Gate_Map.count(instanceName)){ // to std cell
                        Gate_Map[instanceName]->addInput(driving_cell, driving_pin);
                        if(IO_Map.count(driving_cell))
                            IO_Map[driving_cell].addOutput(instanceName, pinName);
                    }
                    else{ // output pin
                        if(IO_Map.count(driving_cell))
                            IO_Map[driving_cell].addOutput(instanceName, pinName);
                    }
                }
            }
        }
    }

    // find largest HPWL output instance
    // find out largest HPWL cell for IO and FF
    for(auto& ff_m : FF_list){
        FF& ff = *ff_m.second;
        const Coor& cur_coor = ff.getOriginalQ(); 
        double maxHPWL = 0;
        const std::vector<std::pair<std::string, std::string>>& output_vector = ff.getOutputInstances();
        for(auto& instance_pair : output_vector){
            const std::string& instance_name = instance_pair.first;
            const std::string& pin_name = instance_pair.second;
            Instance *cur_instance;
            double curHPWL;
            if(FF_list.count(instance_name)){
                cur_instance = FF_list[instance_name];
                curHPWL = HPWL(cur_coor, cur_instance->getCoor()); // in FF list coor is default as D pin coor
            }
            else if(Gate_Map.count(instance_name)){
                cur_instance = Gate_Map[instance_name];
                curHPWL = HPWL(cur_coor, cur_instance->getCoor() + cur_instance->getPinCoor(pin_name));
            }
            else{
                cur_instance = &IO_Map[instance_name];
                curHPWL = HPWL(cur_coor, cur_instance->getCoor());
            }

            if(curHPWL > maxHPWL){
                maxHPWL = curHPWL;
                ff.setLargestOutput(cur_instance, pin_name);
            }
        }
    }

    for(auto& IO_m : IO_Map){
        Instance& IO = IO_m.second;
        const Coor& cur_coor = IO.getCoor();
        double maxHPWL = 0;
        const std::vector<std::pair<std::string, std::string>>& output_vector = IO.getOutputInstances();
        for(auto& instance_pair : output_vector){
            const std::string& instance_name = instance_pair.first;
            const std::string& pin_name = instance_pair.second;
            Instance *cur_instance;
            double curHPWL;
            if(FF_list.count(instance_name)){
                cur_instance = FF_list[instance_name];
                curHPWL = HPWL(cur_coor, cur_instance->getCoor()); // in FF list coor is default as D pin coor
            }
            else if(Gate_Map.count(instance_name)){
                cur_instance = Gate_Map[instance_name];
                curHPWL = HPWL(cur_coor, cur_instance->getCoor() + cur_instance->getPinCoor(pin_name));
            }
            else{
                cur_instance = &IO_Map[instance_name];
                curHPWL = HPWL(cur_coor, cur_instance->getCoor());
            }

            if(curHPWL > maxHPWL){
                maxHPWL = curHPWL;
                IO.setLargestOutput(cur_instance, pin_name);
            }
        }
    }

    // find the largest HPWL input
    // trace the input FF for all FFs in FF list
    for(auto& ff_m : FF_list){
        FF& ff = *ff_m.second;
        std::vector<Instance*> temp_instance;
        std::queue<std::string> q;
        assert(ff.getInputInstances().size() != 0 && "FF with input floating");
        assert(ff.getInputInstances().size() == 1 && "FF should have only one driving cell");
        q.push(ff.getInputInstances()[0].first);
        while(!q.empty()){ // find out all input IO or FF, and save to temp_instance
            std::string cur_instance = q.front();
            q.pop();
            if(FF_list.count(cur_instance)){
                temp_instance.push_back(FF_list[cur_instance]);
            }
            else if(IO_Map.count(cur_instance)){
                temp_instance.push_back(&IO_Map[cur_instance]);
            }
            else{ // is std cell
                Gate& cur_gate = *Gate_Map[cur_instance];
                const std::vector<std::pair<std::string, std::string>>& input_vector = cur_gate.getInputInstances();
                for(size_t i=0;i<input_vector.size();i++)
                    q.push(input_vector[i].first);
            }
        }

        const Coor& cur_coor = ff.getCoor();
        double maxHPWL = 0;
        // find the one instance with largest HPWL
        // max idx of HPWL(temp_instance, temp_instance largestOutput);
        for(auto& instance : temp_instance){
            bool isFF = FF_Map.count(instance->getInstanceName());
            double curHPWL;
            std::pair<Instance*, std::string> outputPair = instance->getLargestOutput();
            std::string outputInstanceName = outputPair.first->getInstanceName();
            Coor outputCellCoor;
            if(FF_list.count(outputInstanceName)){ // output to a FF
                outputCellCoor = FF_list[outputInstanceName]->getOriginalD();
            }
            else if(Gate_Map.count(outputInstanceName)){ // output to std cell
                outputCellCoor = outputPair.first->getCoor() + outputPair.first->getPinCoor(outputPair.second);
            }
            else{ // output to IO
                outputCellCoor = outputPair.first->getCoor();
            }
            if(isFF){
                const Cell& cell = *instance->getCell();
                curHPWL = HPWL(FF_list[FF_list_Map[instance->getInstanceName()]]->getOriginalQ(), outputCellCoor);
                curHPWL = cell.getQpinDelay() + DisplacementDelay * curHPWL;
                if(curHPWL > maxHPWL){
                    ff.setLargestInput(FF_list[FF_list_Map[instance->getInstanceName()]]);
                    maxHPWL = curHPWL;
                }
            }
            else{ // is IO
                curHPWL = HPWL(instance->getCoor(), outputCellCoor);
                if(curHPWL > maxHPWL){
                    ff.setLargestInput(instance);
                    maxHPWL = curHPWL;
                }
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


void Manager::optimal_FF_location(){
    // create FF logic
    std::vector<Coor> c(FF_list.size());
    std::unordered_map<std::string, int> idx_map;
    int i=0;
    for(auto&FF_m : FF_list){ // initial location and set index
        idx_map[FF_m.second->getInstanceName()] = i;
        c[i] = FF_m.second->getCoor();
        i++;
    }

    obj_function obj(*this, idx_map);
    const double kAlpha = 100;
    Gradient optimizer(*this, obj, c, kAlpha, idx_map);

    double WNS = DBL_MAX;
    double TNS = 0;
    double AVS = 0; // average slack
    double MAS = -DBL_MAX; // max slack
    std::cout << "Initial location " << std::endl;
    for(auto& lm : FF_list_Map){
        FF temp = *FF_list[lm.second];
        std::cout << lm.first << " map to logic ff " << temp.getInstanceName() << std::endl;
        std::cout << "With timing slacke : " << temp.getTimingSlack("D") << " And Coor : " << temp.getCoor() << std::endl;
        std::cout << "Original D&Q coordinate : (" << temp.getOriginalD() << ", " << temp.getOriginalQ() << ")" << std::endl;
        std::cout << std::endl;

        double slack = temp.getTimingSlack("D");
        if(slack < WNS)
            WNS = slack;
        if(slack < 0)
            TNS += slack;
        AVS += slack;
        if(slack > MAS)
            MAS = slack;
    }

    std::cout << "Slack statistic before Optimize" << std::endl;
    std::cout << "\tWorst negative slack : " << WNS << std::endl;
    std::cout << "\tTotal negative slack : " << TNS << std::endl;
    std::cout << "\tAverage slack : " << AVS / FF_list.size() << std::endl;
    std::cout << "\tMaximum slack : " << MAS << std::endl;
    for(i=0;i<25;i++){
        std::cout << "step : " << i << std::endl;
        optimizer.Step();
        // CAL new slack
    }

    // map each FF to a single bit cell
    Cell* targetCell = nullptr;
    for(auto& cell_m : cell_library.getCellMap()){
        if(cell_library.isFF(cell_m.first) && cell_m.second->getBits() == 1){
            targetCell = cell_m.second;
            break;
        }
    }
    for(auto& ff_m : FF_list){
        FF* cur_ff = ff_m.second;
        cur_ff->setCell(targetCell);
        cur_ff->setCellName(targetCell->getCellName());
    }
    for(auto& ff_m : FF_list){
        FF* cur_ff = ff_m.second;

        // update slack for new location
        double delta_hpwl = 0;
        double delta_q = 0; // delta q pin delay
        // D pin delta HPWL
        std::string inputInstanceName = cur_ff->getInputInstances()[0].first;
        std::string inputPinName = cur_ff->getInputInstances()[0].second;
        Coor inputCoor;
        if(IO_Map.count(inputInstanceName)){
            inputCoor = IO_Map[inputInstanceName].getCoor();
            double old_hpwl = HPWL(inputCoor, cur_ff->getOriginalD());
            double new_hpwl = HPWL(inputCoor, cur_ff->getCoor() + cur_ff->getPinCoor("D"));
            delta_hpwl += old_hpwl - new_hpwl;
        }
        else if(Gate_Map.count(inputInstanceName)){
            inputCoor = Gate_Map[inputInstanceName]->getCoor() + Gate_Map[inputInstanceName]->getPinCoor(inputPinName);
            double old_hpwl = HPWL(inputCoor, cur_ff->getOriginalD());
            double new_hpwl = HPWL(inputCoor, cur_ff->getCoor() + cur_ff->getPinCoor("D"));
            delta_hpwl += old_hpwl - new_hpwl;
        }
        else{
            inputCoor = FF_list[inputInstanceName]->getOriginalQ();
            double old_hpwl = HPWL(inputCoor, cur_ff->getOriginalD());
            double new_hpwl = HPWL(FF_list[inputInstanceName]->getCoor() + cur_ff->getPinCoor("Q"), cur_ff->getCoor() + cur_ff->getPinCoor("D"));
            delta_hpwl += old_hpwl - new_hpwl;
        }

        // Q pin delta HPWL (prev stage FFs Qpin)
        if(cur_ff->getLargestInput()){
            std::string prevInstanceName = cur_ff->getLargestInput()->getInstanceName();
            Coor originalInput, newInput;
            if(IO_Map.count(prevInstanceName)){
                originalInput = IO_Map[prevInstanceName].getCoor();
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
                if(IO_Map.count(outputInstanceName)){
                    inputCoor = IO_Map[outputInstanceName].getCoor();
                    double old_hpwl = HPWL(inputCoor, originalInput);
                    double new_hpwl = HPWL(inputCoor, newInput);
                    delta_hpwl += old_hpwl - new_hpwl;
                }
                else if(Gate_Map.count(outputInstanceName)){
                    inputCoor = Gate_Map[outputInstanceName]->getCoor() + Gate_Map[outputInstanceName]->getPinCoor(output.second);
                    double old_hpwl = HPWL(inputCoor, originalInput);
                    double new_hpwl = HPWL(inputCoor, newInput);
                    delta_hpwl += old_hpwl - new_hpwl;
                }
                else{
                    inputCoor = FF_list[outputInstanceName]->getOriginalQ();
                    double old_hpwl = HPWL(inputCoor, originalInput);
                    double new_hpwl = HPWL(FF_list[outputInstanceName]->getCoor() + cur_ff->getPinCoor("D"), newInput);
                    delta_hpwl += old_hpwl - new_hpwl;
                }
            }
        }
        // get new slack
        double newSlack = cur_ff->getTimingSlack("D") + (delta_q) + DisplacementDelay * delta_hpwl;
        cur_ff->setTimingSlack("D", newSlack);
    }

    WNS = DBL_MAX;
    TNS = 0;
    AVS = 0; // average slack
    MAS = -DBL_MAX; // max slack
    //
    for(auto& lm : FF_list_Map){
        FF temp = *FF_list[lm.second];
        std::cout << lm.first << " map to logic ff " << temp.getInstanceName() << std::endl;
        std::cout << "With timing slacke : " << temp.getTimingSlack("D") << " And Coor : " << temp.getCoor() << std::endl;
        std::cout << "Original D&Q coordinate : (" << temp.getOriginalD() << ", " << temp.getOriginalQ() << ")" << std::endl;
        std::cout << std::endl;
        
        double slack = temp.getTimingSlack("D");
        if(slack < WNS)
            WNS = slack;
        if(slack < 0)
            TNS += slack;
        AVS += slack;
        if(slack > MAS)
            MAS = slack;
    }

    std::cout << "Slack statistic after Optimize" << std::endl;
    std::cout << "\tWorst negative slack : " << WNS << std::endl;
    std::cout << "\tTotal negative slack : " << TNS << std::endl;
    std::cout << "\tAverage slack : " << AVS / FF_list.size() << std::endl;
    std::cout << "\tMaximum slack : " << MAS << std::endl;
    // */

}

std::string Manager::getNewFFName(const std::string& prefix){
    int count = name_record[prefix];
    assert("number of FF exceed INT_MAX, pls modify counter datatype" && count != INT_MAX);
    name_record[prefix]++;
    return prefix + std::to_string(count);
}