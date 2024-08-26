#include "Checker.h"

Checker::Checker(Manager &mgr) :mgr(mgr){}

Checker::~Checker(){}

void Checker::run(){
    DEBUG_CHECKER("Start Checker");
    initialChecker();
    checkDieBoundary();
    checkOverlap();
    checkOnSite();
    DEBUG_CHECKER("All Check Pass");
}

void Checker::initialChecker(){
    FFs.reserve(mgr.FF_Map.size());
    for(const auto &FF_pair : mgr.FF_Map){
        double startX = FF_pair.second->getNewCoor().x;
        double endX = startX + FF_pair.second->getW();
        double startY = FF_pair.second->getNewCoor().y;
        double endY = startY + FF_pair.second->getH();
        Rect ff;
        ff.startX = startX;
        ff.endX = endX;
        ff.startY = startY;
        ff.endY = endY;
        ff.instName = FF_pair.first;
        FFs.push_back(ff);
    }
    sortRects(FFs);
    Gates.reserve(mgr.Gate_Map.size());
    for(const auto &gate_pair : mgr.Gate_Map){
        double startX = gate_pair.second->getCoor().x;
        double endX = gate_pair.second->getCoor().x + gate_pair.second->getW();
        double startY = gate_pair.second->getCoor().y;
        double endY = gate_pair.second->getCoor().y + gate_pair.second->getH();
        
        Rect gate;
        gate.startX = startX;
        gate.endX = endX;
        gate.startY = startY;
        gate.endY = endY;
        gate.instName = gate_pair.first;
        Gates.push_back(gate);
    }
    sortRects(Gates);
}

void Checker::checkOnSite(){
    DEBUG_CHECKER("Check On Site");
    std::vector<PlacementRow> PlacementRows = mgr.die.getPlacementRows();
    std::sort(PlacementRows.begin(), PlacementRows.end(), [](const PlacementRow& a, const PlacementRow& b){
        if(a.startCoor.y == b.startCoor.y)
            return a.startCoor.x < b.startCoor.y;
        else
            return a.startCoor.y < b.startCoor.y;
    });

    for(size_t i = 0; i < FFs.size(); i++){
        bool isOnSite = false;
        Rect& ff = FFs[i];
        for(size_t j = 0; j < PlacementRows.size(); j++){
            double rowStartX = PlacementRows[j].startCoor.x; 
            double rowEndX = rowStartX + PlacementRows[j].siteWidth * PlacementRows[j].NumOfSites;
            if(ff.startY == PlacementRows[j].startCoor.y){
                if(ff.startX >= rowStartX && ff.startX < rowEndX){
                    double devideResult = (ff.startX - rowStartX) / PlacementRows[j].siteWidth;
                    if(floor(devideResult) == devideResult){
                        isOnSite = true;
                        break;
                    }
                }
            }
            else if(ff.startY < PlacementRows[j].startCoor.y) break;
        }
        if(!isOnSite){
            DEBUG_CHECKER(ff.instName);
            assert(0 && " Not On Site !!");
        }
    }

}

void Checker::checkOverlap(){
    DEBUG_CHECKER("Check Overlap");
    for(size_t i = 0; i < FFs.size() - 1; i++){
        Rect& ff1 = FFs[i];
        for(size_t j = i + 1; j < FFs.size(); j++){
            Rect& ff2 = FFs[j];
            if(ff2.startX > ff1.endX) break;
            if(overlap(ff1, ff2)){
                DEBUG_CHECKER(ff1.instName + " " + ff2.instName);
                assert(0 && " Overlap !!");
            }
        }
        for(size_t j = 0; j < Gates.size(); j++){
            Rect& gate = Gates[j];
            if(gate.startX > ff1.endX) break;
            if(overlap(ff1, gate)){
                DEBUG_CHECKER(ff1.instName + " " + gate.instName);
                assert(0 && " Overlap !!");
            }
        }
    }
}

bool Checker::overlap(Rect rect1, Rect rect2){
    const Rect &rectUp = (rect1.startY > rect2.startY) ? rect1 : rect2;
    const Rect &rectLow = (rect1.startY > rect2.startY) ? rect2 : rect1;
    if(rectUp.startY >= rectLow.endY){
        return false;
    }
    const Rect &rectLeft = (rect1.startX < rect2.startX) ? rect1 : rect2;
    const Rect &rectRight = (rect1.startX < rect2.startX) ? rect2 : rect1;
    if(rectRight.startX >= rectLeft.endX){
        return false;
    }
    return true;
}


void Checker::sortRects(std::vector<Rect> &rect){
    auto rectCmp = [](const Rect &a, const Rect &b){
        if(a.startX != b.startX)
            return a.startX < b.startX;
        else if(a.endX != b.endX)
            return a.endX < b.endX;
        else if(a.startY != b.startY)
            return a.startY < b.startY;
        else
            return a.endY < b.endY;
    };
    std::sort(rect.begin(), rect.end(), rectCmp);
}

void Checker::checkDieBoundary(){
    DEBUG_CHECKER("Check Die Boundary");
    for(size_t i = 0; i < FFs.size() - 1; i++){
        if(FFs[i].startX < mgr.die.getDieOrigin().x || FFs[i].endX > mgr.die.getDieBorder().x){
            DEBUG_CHECKER(FFs[i].instName + ": " + std::to_string(FFs[i].startX) + " " + std::to_string(FFs[i].endX));
            DEBUG_CHECKER("Die X Boundary: " + std::to_string(mgr.die.getDieOrigin().x) + " " + std::to_string(mgr.die.getDieBorder().x));
            assert(0 && " Over x-axis Die Boundary !!");
        }else if(FFs[i].startY < mgr.die.getDieOrigin().y || FFs[i].endY > mgr.die.getDieBorder().y){
            DEBUG_CHECKER(FFs[i].instName + ": " + std::to_string(FFs[i].startY) + " " + std::to_string(FFs[i].endY));
            DEBUG_CHECKER("Die Y Boundary: " + std::to_string(mgr.die.getDieOrigin().y) + " " + std::to_string(mgr.die.getDieBorder().y));
            assert(0 && " Over y-axis Die Boundary !!");
        }
    }

}