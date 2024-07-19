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
    //CheckSubrowsAttribute();     // Should remove when release the binary
    Abacus();
    LegalizeResultWriteBack();
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
    for(const auto &pair : mgr.FF_Map){
        Node *ff = new Node();
        ff->setName(pair.second->getInstanceName());
        ff->setGPCoor(pair.second->getNewCoor());
        ff->setLGCoor(Coor(DBL_MAX, DBL_MAX));
        ff->setW(pair.second->getW());
        ff->setH(pair.second->getH());
        ff->setWeight(pair.second->getPinCount());
        ff->setIsPlace(false);
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
        gate->setIsPlace(false);
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
        subrow->setHeight(PlacementRows[i].siteHeight);
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
    // for each gate, if it occupies a placement row, slice the row
    for(const auto &gate : gates){
        for(auto &row : rows){
            row->slicing(gate);
        }
        gate->setIsPlace(true);
    }
}

void Legalizer::Abacus(){
    // Sort cell by cost function
    std::sort(ffs.begin(), ffs.end(), [](Node *a, Node *b){
        double costA = a->getH() * a->getW() * AREA_WEIGHT + a->getH() * HEIGHT_WEIGHT + a->getW() * WIDTH_WEIGHT + a->getGPCoor().x * X_WEIGHT;
        double costB = b->getH() * b->getW() * AREA_WEIGHT + b->getH() * HEIGHT_WEIGHT + b->getW() * WIDTH_WEIGHT + b->getGPCoor().x * X_WEIGHT;;
        return costA > costB;
    });
    

    DEBUG_LGZ("Start Legalize FF");
    for(const auto &ff : ffs){
        int closest_row_idx = FindClosestRow(ff);
        double minDisplacement = PlaceMultiHeightFFOnRow(ff, closest_row_idx);

        // search down
        int down_row_idx = closest_row_idx - 1;
        while(down_row_idx >= 0 && std::abs(ff->getGPCoor().y - rows[down_row_idx]->getStartCoor().y) < minDisplacement){
            double downDisplacement = PlaceMultiHeightFFOnRow(ff, down_row_idx);
            minDisplacement = minDisplacement < downDisplacement ? minDisplacement : downDisplacement;
            down_row_idx--;
        }

        // search up
        int up_row_idx = closest_row_idx + 1;
        while(up_row_idx < numrows && std::abs(ff->getGPCoor().y - rows[up_row_idx]->getStartCoor().y) < minDisplacement){
            double upDisplacement = PlaceMultiHeightFFOnRow(ff, up_row_idx);
            minDisplacement = minDisplacement < upDisplacement ? minDisplacement : upDisplacement;
            up_row_idx++;
        }

        if(ff->getIsPlace()){
            for(auto &row : rows){
                row->slicing(ff);
            }
        }
        else{
            // Global search mode
            DEBUG_LGZ("Legalize " + ff->getName() + " FF Failed => Enter Greedy Global Search Mode");
            int closest_row_idx = FindClosestRow(ff);
            double minDisplacement = PlaceMultiHeightFFOnRow(ff, closest_row_idx);

            // search down
            int down_row_idx = closest_row_idx - 1;
            while(down_row_idx >= 0){
                double downDisplacement = PlaceMultiHeightFFOnRow(ff, down_row_idx);
                minDisplacement = minDisplacement < downDisplacement ? minDisplacement : downDisplacement;
                down_row_idx--;
            }

            // search up
            int up_row_idx = closest_row_idx + 1;
            while(up_row_idx < numrows){
                double upDisplacement = PlaceMultiHeightFFOnRow(ff, up_row_idx);
                minDisplacement = minDisplacement < upDisplacement ? minDisplacement : upDisplacement;
                up_row_idx++;
            }

            if(ff->getIsPlace()){
                for(auto &row : rows){
                    row->slicing(ff);
                }
            }
            else{
                DEBUG_LGZ("Legalize " + ff->getName() + " FF Failed");
                DEBUG_LGZ("Check if die area or placement row is enough to place...");
                DEBUG_LGZ("Check area parameter is 0 or not...");
            }
        } 
    }
}

// Write legalize coordinate back to manager
void Legalizer::LegalizeResultWriteBack(){
    DEBUG_LGZ("Write Back Legalize Coordinate...");
    for(const auto &ff : ffs){
        if(ff->getIsPlace()){
            // assert((((int)ff->getLGCoor().x - 15300) % 510) == 0);
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
        double placeH;
        if(h >= rows[i]->getSiteHeight()){
            placeH = rows[i]->getSiteHeight();
        }else{
            placeH = h;
        }
        // check if this row can place the cell from startX to startX + w
        if(rows[i]->canPlace(startX, startX + w, placeH)){
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
    DEBUG_LGZ("Check slicing attribute...");
    for(const auto &row: rows){
        for(const auto &subrow : row->getSubrows()){
            assert((int)(subrow->getStartX() - row->getStartCoor().x) % (int)(row->getSiteWidth()) == 0);
            assert(subrow->getStartX() < subrow->getEndX());

            for(const auto &gate: gates){
                if(gate->getGPCoor().y >= row->getStartCoor().y + row->getSiteHeight() || gate->getGPCoor().y + gate->getH() <= row->getStartCoor().y) continue;
                if(subrow->getEndX() > gate->getGPCoor().x && subrow->getStartX() < gate->getGPCoor().x + gate->getW()){
                    std::cout << "slicing fail" << std::endl;
                }
            }

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

    // Should make use of this trend:
    // if the ff->getGPCoor() > subrow->getEndX(), then the displacement will be decreasing
    // if the ff->getGPCoor() < subrow->getStartX() then the displacement will be increasing
    // otherwise, will decrease then increasing

    double minDisplacement = ff->getDisplacement(ff->getLGCoor());

    // Exhausted search to place
    // iterate through subrow in this row
    // [TODO]: find the best entry to the subrow
    // std::cout << "All row: " << *rows[row_idx];


    for(const auto &subrow : rows[row_idx]->getSubrows()){
        // no space in this subrow
        // if(subrow->getFreeWidth() < ff->getW()) continue;
        // for each subrow, try to place on site if has place
        // [TODO]: modify to bisection method
        double alignedStartX = rows[row_idx]->getStartCoor().x + std::ceil((int)(subrow->getStartX() - rows[row_idx]->getStartCoor().x) / rows[row_idx]->getSiteWidth()) * rows[row_idx]->getSiteWidth();
        
        for(int x = alignedStartX; x <= subrow->getEndX(); x += rows[row_idx]->getSiteWidth()){
            // check if upper row can be used...
            bool placeable = ContinousAndEmpty(x, rows[row_idx]->getStartCoor().y, ff->getW(), ff->getH(), row_idx);
            Coor currCoor = Coor(x, rows[row_idx]->getStartCoor().y);
            if(placeable && ff->getDisplacement(currCoor) < minDisplacement){
                // std::cout << "Ori Disp.: " << minDisplacement << ", cur Disp.: " << ff->getDisplacement(currCoor) << std::endl;
                // std::cout << "ff: " << currCoor << ", w = " <<  ff->getW() << std::endl;
                // std::cout << *subrow << std::endl;
                ff->setLGCoor(currCoor);
                minDisplacement = ff->getDisplacement(currCoor);
                ff->setIsPlace(true);
            }
            //if not placeable, increase the step size to escape
            // else{
            //     x += 20 * rows[row_idx]->getSiteWidth();
            // }
        }
    }
    return minDisplacement;
}