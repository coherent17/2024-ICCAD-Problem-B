#include "Dumper.h"

Dumper::Dumper(const std::string &filename){
    fout.open(filename.c_str());
    assert(fout.good());
}

Dumper::~Dumper(){
    fout.close();
}

void Dumper::dump(Manager &mgr){
    fout << "CellInst " << mgr.FF_Map.size() << std::endl;
    // using map to get the consistent order
    std::map<std::string, FF *> ordered_map(mgr.FF_Map.begin(), mgr.FF_Map.end());
    std::unordered_map<std::string, FF*> FF_list = mgr.preprocessor->getFFList();
    std::unordered_map<std::string, std::string> FF_list_Map = mgr.preprocessor->getFFListMap(); // map input MBFF to FF_list, MBFF_NAME/PIN_NAME -> FF_list key

    for(const auto &pair: ordered_map){
        std::string newInstanceName = GenNewInstanceName(pair.second->getInstanceName());
        fout << "Inst " 
             << newInstanceName << " " 
             << pair.second->getCellName() << " " 
             << pair.second->getCoor().x << " " 
             << pair.second->getCoor().y << std::endl;
    }

    for(const auto &pair: mgr.originalFF_Map){
        FF* inputFF = pair.second;
        std::string inputInstance = pair.first;
        for(int i = 0; i < inputFF->getCell()->getPinCount(); i++){
            std::string pinName = inputFF->getCell()->getPinName(i);
            if(pinName[0] == 'D' || pinName[0] == 'Q'){
                std::string MapName = pinName;
                MapName[0] = 'D';
                FF* mapFF = FF_list[FF_list_Map[inputInstance + "/" + MapName]];
                FF* bankFF = mapFF->getPhysicalFF();
                std::string newInstanceName = GenNewInstanceName(bankFF->getInstanceName());
                std::string mapPinName = pinName[0] + mapFF->getPhysicalPinName();

                fout << inputInstance << "/"
                     << pinName << " map "
                     << newInstanceName << "/"
                     << mapPinName << std::endl;
            }
            else{ // CLK net
                FF* mapFF;
                if(inputFF->getCell()->getBits() == 1) // as state in SPEC, only same clk can be clust together, so can map original clk to first D pin clk
                    mapFF = FF_list[FF_list_Map[inputInstance + "/D"]];
                else
                    mapFF = FF_list[FF_list_Map[inputInstance + "/D0"]];
                FF* bankFF = mapFF->getPhysicalFF();
                std::string newInstanceName = GenNewInstanceName(bankFF->getInstanceName());
                fout << inputInstance << "/"
                    << pinName << " map "
                    << newInstanceName << "/"
                    << pinName << std::endl;
            }
        }
    }
}

const std::string Dumper::GenNewInstanceName(const std::string &instanceName){
    return std::string(NEW_INSTANCE_PREFIX + instanceName);
}