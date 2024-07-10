#include "Parser.h"

Parser::Parser(const std::string &filename){
    fin.open(filename.c_str());
    if(!fin.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
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
    std::string _;
    fin >> _ >> mgr.alpha >> _ >> mgr.beta >> _ >> mgr.gamma >> _ >> mgr.lambda;
}

void Parser::readDieBorder(Manager &mgr){
    std::string _;
    double origin_x, origin_y, border_x, border_y;
    fin >> _ >> origin_x >> origin_y >> border_x >> border_y;
    mgr.die.setDieOrigin(Coor(origin_x, origin_y));
    mgr.die.setDieBorder(Coor(border_x, border_y));
}

void Parser::readIOCoordinate(Manager &mgr){
    std::string _;
    std::string pinName;
    double pin_coor_x, pin_coor_y;

    // Input pin
    fin >> _ >> mgr.NumInput;
    for(int i = 0; i < mgr.NumInput; i++){
        fin >> _ >> pinName >> pin_coor_x >> pin_coor_y;
        mgr.Input_Map[pinName] = Coor(pin_coor_x, pin_coor_y);
        Instance input;
        input.setInstanceName(pinName);
        input.setCoor(mgr.Input_Map[pinName]);
        mgr.IO_Map[pinName] = input;
    }

    // Output pin
    fin >> _ >> mgr.NumOutput;
    for(int i = 0; i < mgr.NumOutput; i++){
        fin >> _ >> pinName >> pin_coor_x >> pin_coor_y;
        mgr.Output_Map[pinName] = Coor(pin_coor_x, pin_coor_y);
        Instance output;
        output.setInstanceName(pinName);
        output.setCoor(mgr.Output_Map[pinName]);
        mgr.IO_Map[pinName] = output;
    }
}

void Parser::readCellLibrary(Manager &mgr){
    std::string cellType;
    while(fin >> cellType){
        Cell *c = new Cell();
        int bits = 0;
        int pinCount;
        std::string cellName;
        double w, h;
        if(cellType == "FlipFlop"){
            fin >> bits >> cellName >> w >> h >> pinCount;
            c->setType(Cell_Type::FF);

            if(mgr.MaxBit < bits)
                mgr.MaxBit = bits;

            auto it = mgr.Bit_FF_Map.find(bits);
            if(it != mgr.Bit_FF_Map.end()){
                mgr.Bit_FF_Map[bits].push_back(c);
            }else{
                std::vector<Cell *> cells({c});
                mgr.Bit_FF_Map[bits] = cells;
            }
        }
        else if(cellType == "Gate"){
            fin >> cellName >> w >> h >> pinCount;
            c->setType(Cell_Type::Gate);
        }
        else{
            delete c;
            return;
        };
        c->setBits(bits);
        c->setCellName(cellName);
        c->setW(w);
        c->setH(h);
        c->setPinCount(pinCount);
        std::string _;
        std::string pinName;
        double pin_coor_x, pin_coor_y;
        int inputCount = 0;
        for(int i = 0; i < pinCount; i++){
            fin >> _ >> pinName >> pin_coor_x >> pin_coor_y;
            Coor coor = Coor(pin_coor_x, pin_coor_y);
            c->addPinName(pinName);
            c->addPinCoor(pinName, coor);
            if(pinName.substr(0, 2) == "IN"){
                inputCount++;
            }
        }
        c->setInputCount(inputCount);
        mgr.cell_library.addCell(cellName, c);
    }
}

void Parser::readInstance(Manager &mgr){
    fin >> mgr.NumInstances;
    std::string _;
    std::string instanceName;
    std::string cellType;
    double cell_coor_x, cell_coor_y;
    Coor coor;
    for(int i = 0; i < mgr.NumInstances; i++){
        fin >> _ >> instanceName >> cellType >> cell_coor_x >> cell_coor_y;
        coor = Coor(cell_coor_x, cell_coor_y);
        if(mgr.cell_library.isFF(cellType)){
            FF *ff = new FF();
            ff->setInstanceName(instanceName);
            ff->setCellName(cellType);
            ff->setCoor(coor);
            ff->setCell(mgr.cell_library.getCell(cellType));
            mgr.FF_Map[instanceName] = ff;
        }
        else{
            Gate *gate = new Gate();
            gate->setInstanceName(instanceName);
            gate->setCellName(cellType);
            gate->setCoor(coor);
            gate->setCell(mgr.cell_library.getCell(cellType));
            mgr.Gate_Map[instanceName] = gate;
        }
    }
}

void Parser::readNet(Manager &mgr){
    std::string _;
    fin >> _ >> mgr.NumNets;
    for(int i = 0; i < mgr.NumNets; i++){
        Net net;
        std::string netName;
        int numPins;
        fin >> _ >> netName >> numPins;
        net.setNetName(netName);
        net.setNumPins(numPins);
        for(int j = 0; j < numPins; j++){
            Pin pin;
            std::string pinName;
            fin >> _ >> pinName;
            pin.setIsIOPin(mgr.isIOPin(pinName));
            if(!mgr.isIOPin(pinName)){
                pin.setPinName(getSubStringAfterSlash(pinName));
                pin.setInstanceName(getSubStringBeforeSlash(pinName));
            }
            else{
                pin.setPinName(pinName);
                pin.setInstanceName(pinName);
            }
            net.addPins(pin);
        }
        mgr.Net_Map[netName] = net;
    }
}

void Parser::readBin(Manager &mgr){
    std::string _;
    double BinWidth, BinHeight, BinMaxUtil;
    fin >> _ >> BinWidth;
    fin >> _ >> BinHeight;
    fin >> _ >> BinMaxUtil;
    mgr.die.setBinWidth(BinWidth);
    mgr.die.setBinHeight(BinHeight);
    mgr.die.setBinMaxUtil(BinMaxUtil);
}

void Parser::readPlacementRows(Manager &mgr){
    std::string _;
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
    std::string _, cellName;
    double QpinDelay;
    fin >> mgr.DisplacementDelay;
    while(fin >> _){
        if(_ != "QpinDelay"){
            break;
        }
        fin >> cellName >> QpinDelay;
        mgr.cell_library.getCell(cellName)->setQpinDelay(QpinDelay);
    }
}

void Parser::readTimingSlack(Manager &mgr){
    std::string _, instanceName;
    std::string pinName;
    double TimingSlack;
    do{
        fin >> instanceName >> pinName >> TimingSlack;
        mgr.FF_Map[instanceName]->setTimingSlack(pinName, TimingSlack);
        fin >> _;
    }while(_ == "TimingSlack");
}

void Parser::readGatePower(Manager &mgr){
    std::string _, cellName;
    double GatePower;
    do{
        fin >> cellName >> GatePower;
        mgr.cell_library.getCell(cellName)->setGatePower(GatePower);
        fin >> _;
    }while(_ == "GatePower" && !fin.eof());
}

std::string Parser::getSubStringAfterSlash(const std::string &str){
    size_t pos = str.find_last_of('/');
    if (pos != std::string::npos) {
        return str.substr(pos + 1); // Return substd::string after the last "/"
    }
    // If no "/" found, return the original std::string
    return str;
}

std::string Parser::getSubStringBeforeSlash(const std::string &str){
    size_t pos = str.find_first_of('/');
    if (pos != std::string::npos) {
        return str.substr(0, pos); // Return substd::string before the first "/"
    }
    // If no "/" found, return the original std::string
    return str;
}