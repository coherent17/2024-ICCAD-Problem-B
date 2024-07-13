#include "Legalizer.h"

Legalizer::Legalizer(Manager& mgr) : mgr(mgr){
    numffs = 0;
    numgates = 0;
    numrows = 0;
    minRowHeight = DBL_MAX;
}

Legalizer::~Legalizer(){
    for(auto &ff : ffs){
        delete ff;
    }
    for(auto &gate : gates){
        delete gate;
    }
    for(auto &row : rows){
        delete row;
    }
}

bool Legalizer::run(){
    ConstructDB();
    SliceRows();
    CheckSubrowsAttribute();     // Should remove when release the binary
    Abacus();
    LegalizeResultWriteBack();
    // for(const auto &row : rows){
    //     std::cout << *row << std::endl;
    // }

    for(const auto &ff : ffs){
        if(!ff->getIsPlace()){
            DEBUG_LGZ("Legalization Failed...");
            return false;
        }
    }
    return true;
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
        ff->setLGCoor(Coor(DBL_MAX, DBL_MAX));
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
        gate->setLGCoor(pair.second->getCoor());
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
        minRowHeight = std::min(minRowHeight, PlacementRows[i].siteHeight);
    }

    // sort row by the y coordinate in ascending order, if tie, sort by x in ascending order
    std::sort(rows.begin(), rows.end(), [](Row *a, Row *b){
        return *a < *b;
    });
    
}

void Legalizer::SliceRows(){
    DEBUG_LGZ("Seperate PlacementRows by Gate Cell");
    // will blockage affect the slicing row? It seems not effect...
    // std::sort(gates.begin(), gates.end(), [](Node *node1, Node *node2){
    //     return node1->getGPCoor().x < node2->getGPCoor().x;
    // });

    // for each gate, if it occupies a placement row, slice the row
    for(const auto &gate : gates){
        for(auto &row : rows){
            if(IsOverlap(gate->getGPCoor(), gate->getW(), gate->getH(), row->getStartCoor(), row->getSiteWidth() * row->getNumOfSite(), row->getSiteHeight())){
                // DEBUG_LGZ("Overlap detected...");
                row->slicing(gate);
            }
        }
        gate->setIsPlace(true);
    }
}

void Legalizer::Abacus(){
    // Sort cell by cost function
    std::sort(ffs.begin(), ffs.end(), [](Node *a, Node *b){
        double costA = a->getH() * HEIGHT_WEIGHT + a->getW() * WIDTH_WEIGHT + a->getGPCoor().x * X_WEIGHT;
        double costB = b->getH() * HEIGHT_WEIGHT + b->getW() * WIDTH_WEIGHT + b->getGPCoor().x * X_WEIGHT;;
        return costA > costB;
    });


    // [TODO]:
    // 1. Find the closest place to place the row height > minRowHeight ff and slicing the affect row
    // 2. Run normal abacus algorithm to further place the ff that height <= the minRowHeight
    
    // Find the ff that has multi-row height...

    std::vector<Node *> multiRowHeightFFs;
    std::vector<Node *> normalHeightFFs;
    for(const auto &ff : ffs){
        if(ff->getH() > minRowHeight){
            multiRowHeightFFs.push_back(ff);
        }
        else{
            normalHeightFFs.push_back(ff);
        }
    }

    for(const auto &ff : multiRowHeightFFs){
        int closest_row_idx = FindClosestRow(ff);
        double minDisplacement = PlaceMultiHeightFFOnRow(ff, closest_row_idx);

        // search down until the y displacement is greater than the displacement
        int down_row_idx = closest_row_idx - 1;
        while(down_row_idx >= 0 && std::abs(ff->getGPCoor().y - rows[down_row_idx]->getStartCoor().y) < minDisplacement){
            double downDisplacement = PlaceMultiHeightFFOnRow(ff, down_row_idx);
            minDisplacement = minDisplacement < downDisplacement ? minDisplacement : downDisplacement;
            down_row_idx--;
        }

        // search up
        int up_row_idx = closest_row_idx + 1;
        while(up_row_idx < numrows && std::abs(ff->getGPCoor().y - rows[down_row_idx]->getStartCoor().y) < minDisplacement){
            double upDisplacement = PlaceMultiHeightFFOnRow(ff, down_row_idx);
            minDisplacement = minDisplacement < upDisplacement ? minDisplacement : upDisplacement;
            up_row_idx++;
        }

        // fix the multi row height ff, make it as gate
        if(ff->getIsPlace()){
            for(auto &row : rows){
                if(IsOverlap(ff->getLGCoor(), ff->getW(), ff->getH(), row->getStartCoor(), row->getSiteWidth() * row->getNumOfSite(), row->getSiteHeight())){
                    row->slicing(ff);
                }
            }
        }
        else DEBUG_LGZ("Legalized Multi Row Height FF failed...");
    }

    // Place the normal height FF
    // for(const auto &ff : normalHeightFFs){

    // }


}

// Write legalize coordinate back to manager
void Legalizer::LegalizeResultWriteBack(){
    DEBUG_LGZ("Write Back Legalize Coordinate...");
    for(const auto &ff : ffs){
        if(ff->getIsPlace()){
            mgr.FF_Map[ff->getName()]->setNewCoor(ff->getLGCoor());
        }
        else{
            mgr.FF_Map[ff->getName()]->setNewCoor(Coor(0, 0));
        }
    }
}

bool Legalizer::IsOverlap(const Coor &coor1, double w1, double h1, const Coor &coor2, double w2, double h2){
    // Check if one rectangle is to the left of the other
    if(coor1.x + w1 <= coor2.x || coor2.x + w2 <= coor1.x){
        return false;
    }

    // Check if one rectangle is above the other
    if (coor1.y + h1 <= coor2.y || coor2.y + h2 <= coor1.y) {
            return false;
    }
    return true;
}

// Given the start coordinate from the row, and the w and h,
// find if there exist continous row combination to place a multi-height ff
// Need to take care of row not alignment conditions
bool Legalizer::ContinousAndEmpty(double startX, double startY, double w, double h, int row_idx){
    double endY = startY + h;
    double currentY = startY;

    for(size_t i = row_idx; i < rows.size(); i++){
        double rowStartY = rows[i]->getStartCoor().y;
        double rowEndY = rowStartY + rows[i]->getSiteHeight();

        // check if the current row is complete beyond the target endY
        if(rowStartY >= endY) break;

        // check if this row can place the cell from startX to startX + w
        if(rows[i]->canPlace(startX, startX + w)){
            // Ensure the row covers the currentY to at least part of the target range
            if(rowStartY <= currentY && rowEndY > currentY){
                currentY = rowEndY;
            }

            // Check if we reached the target height
            if(currentY >= endY) return true;
        }
    }

    return false;
}

void Legalizer::CheckSubrowsAttribute(){
    for(const auto &row: rows){
        for(const auto &subrow : row->getSubrows()){
            assert((int)(subrow->getStartX() - row->getStartCoor().x) % (int)(row->getSiteWidth()) == 0);
            assert(subrow->getStartX() < subrow->getEndX());
        }
    }
}

int Legalizer::FindClosestRow(Node *ff){
    double min_distance = DBL_MAX;
    size_t min_idx = 0;
    for(size_t i = 0; i < rows.size(); i++){
        double curr_distance = std::abs(ff->getGPCoor().y - rows[i]->getStartCoor().y);
        if(curr_distance < min_distance){
            min_distance = curr_distance;
            min_idx = i;
        }
    }
    return min_idx;
}


// Try to place on this row, for multi-row height, must check upper row can place or not
// Return the displacement from the global placement coordinate
double Legalizer::PlaceMultiHeightFFOnRow(Node *ff, int row_idx){
    double minDisplacement = ff->getDisplacement(ff->getLGCoor());

    // iterate through subrow in this row
    // [TODO]: find the best entry to the subrow
    for(const auto &subrow : rows[row_idx]->getSubrows()){
        // no space in this subrow
        if(subrow->getFreeWidth() < ff->getW()) continue;

        // for each subrow, try to place on site if has place
        for(int x = subrow->getStartX(); x + ff->getW() < subrow->getEndX(); x += rows[row_idx]->getSiteWidth()){
            // check if upper row can be used...
            bool placeable = ContinousAndEmpty(subrow->getStartX(), rows[row_idx]->getStartCoor().y, ff->getW(), ff->getH(), row_idx);
            Coor currCoor = Coor(x, rows[row_idx]->getStartCoor().y);
            if(placeable && ff->getDisplacement(currCoor) < minDisplacement){
                ff->setLGCoor(currCoor);
                minDisplacement = ff->getDisplacement(currCoor);
                ff->setIsPlace(true);
            }
        }
    }
    return minDisplacement;
}