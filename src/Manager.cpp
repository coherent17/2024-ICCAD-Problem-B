#include "Manager.h"
// #include "OptimalLocation.h"
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
    ;
}

void Manager::Read_InputFile(const std::string &filename){
    Parser parser(filename);
    parser.parse(*this);
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
        std::cout << pair.second << std::endl;
    }

    std::cout << "#################### Gate Instance ##################" << std::endl;
    for(const auto &pair: Gate_Map){
        std::cout << pair.second << std::endl;
    }

    std::cout << "#################### Netlist ##################" << std::endl;
    for(const auto &pair: Net_Map){
        std::cout << pair.second << std::endl;
    }
}

bool Manager::isIOPin(const std::string &pinName){
    if(Input_Map.find(pinName) != Input_Map.end()) return true;
    if(Output_Map.find(pinName) != Output_Map.end()) return true;
    return false;
}

// void Manager::Build_Logic_FF(){
//     // debank and save all the FF in logic_FF;
//     // which is all one bit ff without technology mapping(no cell library)
//     for(auto& mbff_m : FF_Map){
//         FF& cur_ff = mbff_m.second;
//         const string& cur_name = mbff_m.first;
//         const Cell& cur_cell = cur_ff.getCell();
//         for(int i=0;i<cur_ff.getPinCount();i++){
//             string pinName = cur_cell.getPinName(i);
//             if(pinName[0] == 'D'){ // Assume all ff D pin start with D
//                 FF temp;
//                 Coor c = cur_ff.getPinCoor(cur_cell.getPinName(i)) + cur_ff.getCoor();
//                 double slack = cur_ff.getTimingSlack(pinName);
//                 temp.setInstanceName(getNewFFName("FF_", FF_list.size()));
//                 temp.setCoor(c);
//                 temp.setTimingSlack(slack, "D0");
//                 FF_list[temp.getInstanceName()] = temp;
//                 FF_list_Map[cur_name+'/'+pinName] = temp.getInstanceName();
//             }
//         }
//     }

//     cout << "Total number of FF : " << FF_list.size() << endl;
//     ///* for debug
//     for(auto& lm : FF_list_Map){
//         FF temp = FF_list[lm.second];
//         cout << lm.first << " map to logic ff " << temp.getInstanceName() << endl;
//         cout << "With timing slacke : " << temp.getTimingSlack("D0") << " And Coor : " << temp.getCoor() << endl;
//         cout << endl;
//     }
//     //*/
// }

// void Manager::Build_Circuit_Gragh(){
//     for(auto& n_m : Net_Map){
//         const Net& n = n_m.second;
//         string driving_cell;
//         string driving_pin;
//         bool is_CLK = false; // ignore clock net (maybe can done in parser)
//         bool has_driving_cell = false;
//         // if(n.getNumPins() == 1) // ignore single pin nets
//         //     continue;
//         for(int i=0;i<n.getNumPins();i++){ // find driving cell of this net
//             // from testcase release it seems driving cell is first pin of net, but not confirm by ICCAD
//             const Pin& p = n.getPin(i);
//             const string& pinName = p.getPinName();
//             if(pinName.substr(0, 3) == "CLK"){
//                 is_CLK = true;
//                 break;
//             }
//             if(pinName[0] == 'Q' || (!p.getIsIOPin() && pinName.substr(0,3) == "OUT") || pinName.substr(0,5) == "INPUT"){
//                 driving_cell = p.getInstanceName();
//                 driving_pin = p.getPinName();
//                 has_driving_cell = true;
//                 break;
//             }
//         }
//         if(is_CLK || !has_driving_cell)
//             continue;

//         for(int i=0;i<n.getNumPins();i++){ // build gragh
//             const Pin& p = n.getPin(i);
//             const string& instanceName = p.getInstanceName();
//             if(instanceName == driving_cell)
//                 continue;
//             else{
//                 if(p.getPinName().substr(0, 3) == "CLK"){
//                     break;
//                 }
//                 // set output cells for driving cell(only for FF)
//                 if(FF_Map.count(driving_cell)){ // drive by FF
//                     string driving_ff = driving_cell + "/D" + driving_pin.substr(1, driving_pin.size()-1);
//                     driving_ff = FF_list[FF_list_Map[driving_ff]].getInstanceName();
//                     if(FF_Map.count(instanceName)){ // to FF
//                         FF_list[FF_list_Map[instanceName + '/' + p.getPinName()]].addInput(driving_ff);
//                         FF_list[driving_ff].addOutput(FF_list[FF_list_Map[instanceName + '/' + p.getPinName()]].getInstanceName());
//                     }
//                     else if(Gate_Map.count(instanceName)){ // to std cell
//                         Gate_Map[instanceName].addInput(driving_ff);
//                         FF_list[driving_ff].addOutput(instanceName);
//                     }
//                     else{ // output pin
//                         FF_list[driving_ff].addOutput(instanceName);
//                     }
//                 }
//                 else{ // drive by std cell or IO
//                     if(FF_Map.count(instanceName)){ // to FF
//                         FF_list[FF_list_Map[instanceName + '/' + p.getPinName()]].addInput(driving_cell);
//                         if(IO_Map.count(driving_cell))
//                             IO_Map[driving_cell].addOutput(FF_list[FF_list_Map[instanceName + '/' + p.getPinName()]].getInstanceName());
//                     }
//                     else if(Gate_Map.count(instanceName)){ // to std cell
//                         Gate_Map[instanceName].addInput(driving_cell);
//                         if(IO_Map.count(driving_cell))
//                             IO_Map[driving_cell].addOutput(instanceName);
//                     }
//                     else{ // output pin
//                         if(IO_Map.count(driving_cell))
//                             IO_Map[driving_cell].addOutput(instanceName);
//                     }
//                 }
//             }
//         }
//     }

//     // find out largest HPWL cell for IO and FF
//     for(auto& ff_m : FF_list){
//         FF& ff = ff_m.second;
//         ff.setLargestOutput(nullptr);
//         const Coor& cur_coor = ff.getCoor();
//         double maxHPWL = 0;
//         const vector<string>& output_vector = ff.getOutput();
//         for(auto& instance_name : output_vector){
//             Instance *cur_instance;
//             if(FF_list.count(instance_name)){
//                 cur_instance = &FF_list[instance_name];
//             }
//             else if(Gate_Map.count(instance_name)){
//                 cur_instance = &Gate_Map[instance_name];
//             }
//             else{
//                 cur_instance = &IO_Map[instance_name];
//             }
//             double curHPWL = HPWL(cur_coor, cur_instance->getCoor());
//             if(curHPWL > maxHPWL){
//                 maxHPWL = curHPWL;
//                 ff.setLargestOutput(cur_instance);
//             }
//         }
//     }

//     for(auto& IO_m : IO_Map){
//         Instance& IO = IO_m.second;
//         IO.setLargestOutput(nullptr);
//         const Coor& cur_coor = IO.getCoor();
//         double maxHPWL = 0;
//         const vector<string>& output_vector = IO.getOutput();
//         for(auto& instance_name : output_vector){
//             Instance *cur_instance;
//             if(FF_list.count(instance_name)){
//                 cur_instance = &FF_list[instance_name];
//             }
//             else if(Gate_Map.count(instance_name)){
//                 cur_instance = &Gate_Map[instance_name];
//             }
//             else{
//                 cur_instance = &IO_Map[instance_name];
//             }
//             double curHPWL = HPWL(cur_coor, cur_instance->getCoor());
//             if(curHPWL > maxHPWL){
//                 maxHPWL = curHPWL;
//                 IO.setLargestOutput(cur_instance);
//             }
//         }
//     }


//     // trace the input FF for all FFs in FF list
//     for(auto& ff_m : FF_list){
//         FF& ff = ff_m.second;
//         vector<Instance*> temp_instance;
//         queue<string> q;
//         assert(ff.getInput().size() != 0 && "FF with input floating");
//         q.push(ff.getInput()[0]);
//         while(!q.empty()){ // find out all input IO or FF, and save to temp_instance
//             string cur_instance = q.front();
//             q.pop();
//             if(FF_list.count(cur_instance)){
//                 temp_instance.push_back(&FF_list[cur_instance]);
//             }
//             else if(IO_Map.count(cur_instance)){
//                 temp_instance.push_back(&IO_Map[cur_instance]);
//             }
//             else{ // is std cell
//                 Gate& cur_gate = Gate_Map[cur_instance];
//                 const vector<string>& input_vector = cur_gate.getInput();
//                 for(size_t i=0;i<input_vector.size();i++)
//                     q.push(input_vector[i]);
//             }
//         }

//         const Coor& cur_coor = ff.getCoor();
//         double maxHPWL = 0;
//         ff.setLargestInput(nullptr);
//         // find the one instance with largest HPWL
//         for(auto& instance : temp_instance){
//             bool isFF = FF_Map.count(instance->getInstanceName());
//             double curHPWL = HPWL(cur_coor, instance->getCoor());
//             if(isFF){
//                 const Cell& cell = instance->getCell();
//                 curHPWL = cell.getQpinDelay() + DisplacementDelay * curHPWL;
//             }
//             if(curHPWL > maxHPWL){
//                 ff.setLargestInput(instance);
//                 maxHPWL = curHPWL;
//             }
//         }
//     }
    
//     // for debug
//     for(auto& lm : FF_list_Map){
//         FF temp = FF_list[lm.second];
//         cout << lm.first << " map to logic ff " << temp.getInstanceName() << endl;
//         if(temp.getLargestInput() != nullptr)
//             cout << "Input largest cell : " << temp.getLargestInput()->getInstanceName() << endl;
//         if(temp.getLargestOutput() != nullptr)
//             cout << "Output largest cell : " << temp.getLargestOutput()->getInstanceName() << endl << endl; 
//         else
//             cout << "Output is floating" << endl;
//     }
//     /*
//     for(auto& lm : FF_list_Map){
//         FF temp = FF_list[lm.second];
//         cout << lm.first << " map to logic ff " << temp.getInstanceName() << endl;
//         cout << "Its input from " << temp.getInput()[0] << endl;
//         if(temp.getOutput().size())
//             for(size_t i=0;i<temp.getOutput().size();i++)
//                 cout << "Its output to " << temp.getOutput()[i] << endl;
//         else
//             cout << "Its output is floating" << endl;
//     }
//     */
// }


// void Manager::optimal_FF_location(){
//     // create FF logic
//     vector<Coor> c(FF_Map.size());
//     int i=0;
//     for(auto&FF_m : FF_Map){ // initial location and set index
//         FF_m.second.setIdx(i);
//         c[i].x = FF_m.second.getCoor().x;
//         c[i].y = FF_m.second.getCoor().y;
//         i++;
//     }

//     obj_function obj(*this);
//     const double kAlpha = 0.01;
//     Gradient optimizer(obj, c, kAlpha);

//     for(i=0;i<25;i++){
//         optimizer.Step();
//         // CAL new slack

//     }
// }