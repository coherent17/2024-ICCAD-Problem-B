#include "Manager.h"
Manager::Manager():
    alpha(0),
    beta(0),
    gamma(0),
    lambda(0),
    DisplacementDelay(0),
    NumInput(0),
    NumOutput(0),
    NumInstances(0),
    NumNets(0),
    preprocessor(new Preprocess(*this))
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

void Manager::preprocess(){
    preprocessor->run();
    FF_Map = preprocessor->getFFList();
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

    fout << "DieSize " << die.getDieOrigin().x << " " << die.getDieOrigin().y << " " << die.getDieBorder().x << " " << die.getDieBorder().y << std::endl;

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


    // for cell library
    std::unordered_map<std::string, Cell *> cellMap = cell_library.getCellMap();
    for(const auto &pair: cellMap){
        if(pair.second->getType() == Cell_Type::FF){
            fout << "FlipFlop " << pair.second->getBits() << " " << pair.second->getCellName() << " " << pair.second->getW() << " " << pair.second->getH() << " " << pair.second->getPinCount() << std::endl;
            std::unordered_map<std::string, Coor> pinCoorMap= pair.second->getPinCoorMap();
            for(const auto &p : pinCoorMap){
                fout << "Pin " << p.first << " " << p.second.x << " " << p.second.y << std::endl;
            }
        }
        else if(pair.second->getType() == Cell_Type::Gate){
            fout << "Gate " << pair.second->getCellName() << " " << pair.second->getW() << " " << pair.second->getH() << " " << pair.second->getPinCount() << std::endl;
            std::unordered_map<std::string, Coor> pinCoorMap= pair.second->getPinCoorMap();
            for(const auto &p : pinCoorMap){
                fout << "Pin " << p.first << " " << p.second.x << " " << p.second.y << std::endl;
            }
        }
        else{
            abort();
        }
    }

    fout << "NumInstances " << FF_Map.size() + Gate_Map.size() << std::endl;
    std::map<std::string, FF *> ff_map(FF_Map.begin(), FF_Map.end());
    for(const auto &pair: ff_map){
        fout << "Inst " << pair.first << " " << pair.second->getCellName() << " " << pair.second->getNewCoor().x << " " << pair.second->getNewCoor().y << std::endl;
    }
    std::map<std::string, Gate *> gate_map(Gate_Map.begin(), Gate_Map.end());
    for(const auto &pair: gate_map){
        fout << "Inst " << pair.first << " " << pair.second->getCellName() << " " << pair.second->getCoor().x << " " << pair.second->getCoor().y << std::endl;
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

    fout << "BinWidth " << die.getBinWidth() << std::endl;
    fout << "BinHeight " << die.getBinHeight() << std::endl;
    fout << "BinMaxUtil " << die.getBinMaxUtil() << std::endl;
    std::vector<PlacementRow> pr = die.getPlacementRows();
    for(size_t i = 0; i < pr.size(); i++){
        fout << "PlacementRows " << pr[i].startCoor.x << " " << pr[i].startCoor.y << " " << pr[i].siteWidth << " " << pr[i].siteHeight << " " << pr[i].NumOfSites << std::endl;
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

std::string Manager::getNewFFName(const std::string& prefix){
    int count = name_record[prefix];
    assert("number of FF exceed INT_MAX, pls modify counter datatype" && count != INT_MAX);
    name_record[prefix]++;
    return prefix + std::to_string(count);
}