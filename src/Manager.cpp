#include "Manager.h"

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

void Manager::print(){
    cout << alpha << " " << beta << " " << gamma << " " << lambda << endl;
    cout << DieOrigin << " " << DieBorder << endl;
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
}