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
    std::map<std::string, FF> ordered_map(mgr.FF_Map.begin(), mgr.FF_Map.end());
    for(const auto &pair: ordered_map){
        std::string newInstanceName = GenNewInstanceName(pair.second.getInstanceName());
        fout << "Inst " 
             << newInstanceName << " " 
             << pair.second.getCellName() << " " 
             << pair.second.getCoor().x << " " 
             << pair.second.getCoor().y << std::endl;
    }

    for(const auto &pair: ordered_map){
        std::string newInstanceName = GenNewInstanceName(pair.second.getInstanceName());
        for(int i = 0; i < pair.second.getCell()->getPinCount(); i++){
            fout << pair.second.getInstanceName() << "/"
                 << pair.second.getCell()->getPinName(i) << " map "
                 << newInstanceName << "/"
                 << pair.second.getCell()->getPinName(i) << std::endl;
        }
    }
}

const std::string Dumper::GenNewInstanceName(const std::string &instanceName){
    return std::string(NEW_INSTANCE_PREFIX + instanceName);
}