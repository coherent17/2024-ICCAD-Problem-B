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
    readInstance(mgr);
    readNet(mgr);
    readBin(mgr);
    readPlacementRows(mgr);
    readQpinDelay(mgr);
    readTimingSlack(mgr);
    readGatePower(mgr);
}

void Parser::readWeight(Manager &mgr){
    string _;
    fin >> _ >> mgr.alpha >> _ >> mgr.beta >> _ >> mgr.gamma >> _ >> mgr.lambda;
}

void Parser::readDieBorder(Manager &mgr){
    string _;
    double origin_x, origin_y, border_x, border_y;
    fin >> _ >> origin_x >> origin_y >> border_x >> border_y;
    Coor DieOrigin = Coor(origin_x, origin_y);
    Coor DieBorder = Coor(border_x, border_y);
    mgr.die.setDieOrigin(DieOrigin);
    mgr.die.setDieBorder(DieBorder);
}

void Parser::readIOCoordinate(Manager &mgr){
    string _;
    string pinName;
    double pin_coor_x, pin_coor_y;

    // Input pin
    fin >> _ >> mgr.NumInput;
    for(int i = 0; i < mgr.NumInput; i++){
        fin >> _ >> pinName >> pin_coor_x >> pin_coor_y;
        mgr.Input_Map[pinName] = Coor(pin_coor_x, pin_coor_y);
    }

    // Output pin
    fin >> _ >> mgr.NumOutput;
    for(int i = 0; i < mgr.NumOutput; i++){
        fin >> _ >> pinName >> pin_coor_x >> pin_coor_y;
        mgr.Output_Map[pinName] = Coor(pin_coor_x, pin_coor_y);
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
            Coor coor = Coor(pin_coor_x, pin_coor_y);
            c.addPinCoor(pinName, coor);
        }
        mgr.cell_library.addCell(cellName, c);
    }
}

void Parser::readInstance(Manager &mgr){
    fin >> mgr.NumInstances;
    string _, instanceName, cellType;
    double cell_coor_x, cell_coor_y;
    Coor coor;
    for(int i = 0; i < mgr.NumInstances; i++){
        fin >> _ >> instanceName >> cellType >> cell_coor_x >> cell_coor_y;
        coor = Coor(cell_coor_x, cell_coor_y);
        if(mgr.cell_library.isFF(cellType)){
            FF ff;
            ff.setInstanceName(instanceName);
            ff.setCellName(cellType);
            ff.setCoor(coor);
            //ff.setCellLibraryPtr(mgr.cell_library.getCell(cellType));
            mgr.FF_Map[instanceName] = ff;
        }
        else{
            Gate gate;
            gate.setInstanceName(instanceName);
            gate.setCellName(cellType);
            gate.setCoor(coor);
            //gate.setCellLibraryPtr(mgr.cell_library.getCell(cellType));
            mgr.Gate_Map[instanceName] = gate;
        }
    }
}

void Parser::readNet(Manager &mgr){
    string _;
    fin >> _ >> mgr.NumNets;
    for(int i = 0; i < mgr.NumNets; i++){
        Net net;
        string netName;
        int numPins;
        fin >> _ >> netName >> numPins;
        net.setNetName(netName);
        net.setNumPins(numPins);
        for(int j = 0; j < numPins; j++){
            Pin pin;
            string pinName;
            fin >> _ >> pinName;
            pin.setIsIOPin(mgr.isIOPin(pinName));
            if(!mgr.isIOPin(pinName)){
                pin.setPinName(getSubStringAfterSlash(pinName));
                pin.setInstanceName(getSubStringBeforeSlash(pinName));
            }
            else{
                pin.setPinName(pinName);
            }
            net.addPins(pin);
        }
        mgr.Net_Map[netName] = net;
    }
}

void Parser::readBin(Manager &mgr){
    string _;
    double BinWidth, BinHeight, BinMaxUtil;
    fin >> _ >> BinWidth;
    fin >> _ >> BinHeight;
    fin >> _ >> BinMaxUtil;
    mgr.die.setBinWidth(BinWidth);
    mgr.die.setBinHeight(BinHeight);
    mgr.die.setBinMaxUtil(BinMaxUtil);
}

void Parser::readPlacementRows(Manager &mgr){
    string _;
    while(fin >> _){
        if(_ != "PlacementRows"){
            break;
        }
        PlacementRow placementrow;
        double startX, startY, siteWidth, siteHeight;
        int NumOfSites;
        fin >> startX >> startY >> siteWidth >> siteHeight >> NumOfSites;
        Coor startCoor = Coor(startX, startY);
        placementrow.startCoor = startCoor;
        placementrow.siteWidth = siteWidth;
        placementrow.siteHeight = siteHeight;
        placementrow.NumOfSites = NumOfSites;
        mgr.die.addPlacementRow(placementrow);
    }
}

void Parser::readQpinDelay(Manager &mgr){
    string _, cellName;
    double QpinDelay;
    fin >> mgr.DisplacementDelay;
    while(fin >> _){
        if(_ != "QpinDelay"){
            break;
        }
        fin >> cellName >> QpinDelay;
        mgr.cell_library.getCellRef(cellName).setQpinDelay(QpinDelay);
    }
}

void Parser::readTimingSlack(Manager &mgr){
    string _, instanceName;
    double TimingSlack;
    do{
        fin >> instanceName >> _ >> TimingSlack;
        mgr.FF_Map[instanceName].setTimingSlack(TimingSlack);
        fin >> _;
    }while(_ == "TimingSlack");
}

void Parser::readGatePower(Manager &mgr){
    string _, cellName;
    double GatePower;
    do{
        fin >> cellName >> GatePower;
        mgr.cell_library.getCellRef(cellName).setGatePower(GatePower);
        fin >> _;
    }while(_ == "GatePower" && !fin.eof());
}

string Parser::getSubStringAfterSlash(const string &str){
    size_t pos = str.find_last_of('/');
    if (pos != std::string::npos) {
        return str.substr(pos + 1); // Return substring after the last "/"
    }
    // If no "/" found, return the original string
    return str;
}

string Parser::getSubStringBeforeSlash(const string &str){
    size_t pos = str.find_first_of('/');
    if (pos != std::string::npos) {
        return str.substr(0, pos); // Return substring before the first "/"
    }
    // If no "/" found, return the original string
    return str;
}