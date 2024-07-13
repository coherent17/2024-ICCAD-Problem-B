#include "Legalizer.h"

Legalizer::Legalizer(Manager& mgr) : mgr(mgr){}

Legalizer::~Legalizer(){

}

void Legalizer::run(){
    ConstructDB();
    SliceRows();
    for(const auto &row : rows){
        std::cout << *row << std::endl;
    }
}

void Legalizer::ConstructDB(){
    LoadFF();
    LoadGate();
    LoadPlacementRow();
    DEBUG_LGZ("Finish Load Databse to Legalizer");
}

void Legalizer::LoadFF(){
    // Construct FF database
    DEBUG_LGZ("Load FF to Databse");
    numffs = mgr.FF_Map.size();
    assert(mgr.FFs.size() == mgr.FF_Map.size());
    for(size_t i = 0; i < mgr.FFs.size(); i++){
        Node *ff = new Node();
        ff->setName(mgr.FFs[i]->getInstanceName());
        ff->setGPCoor(mgr.FFs[i]->getNewCoor());
        ff->setLGZCoor(mgr.FFs[i]->getNewCoor());
        ff->setW(mgr.FFs[i]->getW());
        ff->setH(mgr.FFs[i]->getH());
        ff->setWeight(mgr.FFs[i]->getPinCount());
        ffs.push_back(ff);
    }
}

void Legalizer::LoadGate(){
    // Construct Gate database
    DEBUG_LGZ("Load Gate to Databse");
    numgates = mgr.Gate_Map.size();
    for(const auto &pair: mgr.Gate_Map){
        Node *gate = new Node();
        gate->setName(pair.second->getInstanceName());
        gate->setGPCoor(pair.second->getCoor());
        gate->setLGZCoor(pair.second->getCoor());
        gate->setW(pair.second->getW());
        gate->setH(pair.second->getH());
        gate->setWeight(pair.second->getPinCount());
        gates.push_back(gate);
    }
}

void Legalizer::LoadPlacementRow(){
    // Construct Placement row database
    DEBUG_LGZ("Load Placement Row to Databse");
    std::vector<PlacementRow> PlacementRows = mgr.die.getPlacementRows();
    for(size_t i = 0; i < PlacementRows.size(); i++){
        Row *row = new Row();
        row->setStartCoor(PlacementRows[i].startCoor);
        row->setSiteHeight(PlacementRows[i].siteHeight);
        row->setSiteWidth(PlacementRows[i].siteWidth);
        row->setNumOfSite(PlacementRows[i].NumOfSites);
        // init the first subrow in row class
        Subrow *subrow = new Subrow();
        subrow->setStartX(PlacementRows[i].startCoor.x);
        subrow->setEndX(PlacementRows[i].startCoor.x + PlacementRows[i].siteWidth * PlacementRows[i].NumOfSites);
        subrow->setFreeWidth(PlacementRows[i].siteWidth * PlacementRows[i].NumOfSites);
        subrow->setLastCluster(nullptr);
        row->addSubrows(subrow);
        rows.push_back(row);
    }

    // sort row by the y coordinate in ascending order, if tie, sort by x in ascending order
    std::sort(rows.begin(), rows.end(), [](Row *a, Row *b){
        return *a < *b;
    });
    
}

void Legalizer::SliceRows(){
    DEBUG_LGZ("Seperate PlacementRows by Gate Cell");
    std::sort(gates.begin(), gates.end(), [](Node *node1, Node *node2){
        return node1->getGPCoor().x < node2->getGPCoor().x;
    });

    // for(size_t i = 0; i < gates.size(); i++){
    //     DEBUG_LGZ(gates[i]->getName() + "/" + std::to_string(gates[i]->getGPCoor().x));
    // }

    // for each gate, if it occupies a placement row, slice the row
    // for(const auto &gate : gates){
    //     for(auto &row : rows){
    //         if(IsOverlap(gate->getGPCoor(), gate->getW(), gate->getH(), row->getStartCoor(), row->getSiteWidth() * row->getNumOfSite(), row->getSiteHeight())){
    //             DEBUG_LGZ("Overlap detected...");
    //             row->slicing(gate);
    //         }
    //     }
    // }
}

bool Legalizer::IsOverlap(const Coor &coor1, double w1, double h1, const Coor &coor2, double w2, double h2){
    using namespace boost::geometry;
    typedef model::point<double, 2, cs::cartesian> point;
    typedef model::box<point> box;
    box box1(point(coor1.x, coor1.y), point(coor1.x + w1, coor1.y + h1));
    box box2(point(coor2.x, coor2.y), point(coor2.x + w2, coor2.y + h2));
    return intersects(box1, box2);
}