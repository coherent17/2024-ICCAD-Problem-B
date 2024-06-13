#include "Manager.h"
#include "OptimalLocation.h"
Manager::Manager(){
    ;
}

Manager::~Manager(){
    ;
}

void Manager::Read_InputFile(const string &filename){
    Parser parser(filename);
    parser.parse(*this);
}

void Manager::Technology_Mapping(){
    for(auto &pair : FF_Map) {
        const string &cellName = pair.second.getCellName();
        pair.second.setCell(cell_library.getCellCopy(cellName));
    }
    for(auto &pair : Gate_Map) {
        const string &cellName = pair.second.getCellName();
        pair.second.setCell(cell_library.getCellCopy(cellName));
    }
}

void Manager::print(){
    cout << alpha << " " << beta << " " << gamma << " " << lambda << endl;
    cout << "#################### Die Info ##################" << endl;
    cout << die << endl;
    for(const auto &pair : Input_Map){
        cout << pair.first << ":" << pair.second << endl;
    }
    for(const auto &pair : Output_Map){
        cout << pair.first << ":" << pair.second << endl;
    }

    cout << "#################### Cell Library ##################" << endl;
    cout << cell_library << endl;

    cout << "#################### FF Instance ##################" << endl;
    for(const auto &pair: FF_Map){
        cout << pair.second << endl;
    }

    cout << "#################### Gate Instance ##################" << endl;
    for(const auto &pair: Gate_Map){
        cout << pair.second << endl;
    }

    cout << "#################### Netlist ##################" << endl;
    for(const auto &pair: Net_Map){
        cout << pair.second << endl;
    }
}

bool Manager::isIOPin(const string &pinName){
    if(Input_Map.find(pinName) != Input_Map.end()) return true;
    if(Output_Map.find(pinName) != Output_Map.end()) return true;
    return false;
}

void Manager::optimal_FF_location(){
    // create FF logic
    vector<Coor> c(FF_Map.size());
    int i=0;
    for(auto&FF_m : FF_Map){ // initial location and set index
        FF_m.second.setIdx(i);
        c[i].x = FF_m.second.getCoor().x;
        c[i].y = FF_m.second.getCoor().y;
        i++;
    }

    obj_function obj(*this);
    const double kAlpha = 0.01;
    Gradient optimizer(obj, c, kAlpha);

    for(i=0;i<25;i++){
        optimizer.Step();
        // CAL new slack

    }
}