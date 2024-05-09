#include "Parser.h"

Parser::Parser(const string &filename){
    fin.open(filename.c_str());
    assert(fin.good());
}

Parser::~Parser(){
    fin.close();
}

void Parser::parse(Manager &mgr){
    readWeight(mgr);
    readDieBorder(mgr);
    readIOCoordinate(mgr);
    readCellLibrary(mgr);
}

void Parser::readWeight(Manager &mgr){
    string _;
    fin >> _ >> mgr.alpha >> _ >> mgr.beta >> _ >> mgr.gamma >> _ >> mgr.lambda;
}

void Parser::readDieBorder(Manager &mgr){
    string _;
    double origin_x, origin_y, border_x, border_y;
    fin >> _ >> origin_x >> origin_y >> border_x >> border_y;
    mgr.DieOrigin = make_pair(origin_x, origin_y);
    mgr.DieBorder = make_pair(border_x, border_y);
}

void Parser::readIOCoordinate(Manager &mgr){
    string _;
    string pinName;
    double pin_coor_x, pin_coor_y;

    // Input pin
    fin >> _ >> mgr.NumInput;
    for(int i = 0; i < mgr.NumInput; i++){
        fin >> _ >> pinName >> pin_coor_x >> pin_coor_y;
        mgr.Input_Map[pinName] = make_pair(pin_coor_x, pin_coor_y);
    }

    // Output pin
    fin >> _ >> mgr.NumOutput;
    for(int i = 0; i < mgr.NumOutput; i++){
        fin >> _ >> pinName >> pin_coor_x >> pin_coor_y;
        mgr.Output_Map[pinName] = make_pair(pin_coor_x, pin_coor_y);
    }
}

void Parser::readCellLibrary(Manager &mgr){
    string cellType;
    while(fin >> cellType){
        Cell c;
        int bits = 0;
        int pinCount;
        string cellName;
        double w, h;
        if(cellType == "FlipFlop"){
            fin >> bits >> cellName >> w >> h >> pinCount;
            c.setIsFF(true);
        }
        else if(cellType == "Gate"){
            fin >> cellName >> w >> h >> pinCount;
            c.setIsFF(false);
        }
        else return;
        c.setBits(bits);
        c.setCellName(cellName);
        c.setW(w);
        c.setH(h);
        c.setPinCount(pinCount);
        string _, pinName;
        double pin_coor_x, pin_coor_y;
        for(int i = 0; i < pinCount; i++){
            fin >> _ >> pinName >> pin_coor_x >> pin_coor_y;
            Coor coor = make_pair(pin_coor_x, pin_coor_y);
            c.addPinCoor(pinName, coor);
        }
        mgr.cell_library.addCell(cellName, c);
    }
}