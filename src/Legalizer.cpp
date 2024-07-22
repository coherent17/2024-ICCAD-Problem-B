#include "Legalizer.h"

Legalizer::Legalizer(Manager& mgr) : mgr(mgr){
    numffs = 0;
    numgates = 0;
    timer.start();
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
    SliceRowsByRows();
    SliceRowsByGate();
    Abacus();
    LegalizeResultWriteBack();
    for(const auto &ff : ffs){
        if(!ff->getIsPlace()){
            DEBUG_LGZ("Legalization Failed...");
            timer.stop();
            return false;
        }
    }
    timer.stop();
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
        row->setSiteHeight(mgr.die.getDieBorder().y - PlacementRows[i].startCoor.y);
        row->setSiteWidth(PlacementRows[i].siteWidth);
        row->setNumOfSite(PlacementRows[i].NumOfSites);
        double endX = PlacementRows[i].startCoor.x + PlacementRows[i].siteWidth * PlacementRows[i].NumOfSites;
        row->setEndX(endX);
        // init the first subrow in row class
        Subrow *subrow = new Subrow();
        subrow->setStartX(PlacementRows[i].startCoor.x);
        subrow->setEndX(PlacementRows[i].startCoor.x + PlacementRows[i].siteWidth * PlacementRows[i].NumOfSites);
        subrow->setFreeWidth(PlacementRows[i].siteWidth * PlacementRows[i].NumOfSites);
        subrow->setHeight(mgr.die.getDieBorder().y - PlacementRows[i].startCoor.y);
        row->addSubrows(subrow);
        rows.push_back(row);
    }

    // sort row by the y coordinate in ascending order, if tie, sort by x in ascending order
    std::sort(rows.begin(), rows.end(), [](Row *a, Row *b){
        return *a < *b;
    });
    
}

void Legalizer::SliceRowsByRows(){
    for(size_t i = 0; i < rows.size()-1; i++){
        double startX = rows[i]->getStartCoor().x;
        double endX = rows[i]->getEndX();
        std::list<XTour> xList;
        double siteHeight = rows[i]->getSiteHeight();
        for(size_t j = i + 1; j < rows.size(); j++){
            if(rows[j]->getEndX() <= startX || rows[j]->getStartCoor().x >= endX) continue;
            else{
                Node * upRow = new Node();
                upRow->setGPCoor(rows[j]->getStartCoor());
                upRow->setLGCoor(rows[j]->getStartCoor());
                upRow->setW(rows[j]->getEndX() - rows[j]->getStartCoor().x);
                upRow->setH(rows[j]->getSiteHeight());
                rows[i]->slicing(upRow);
                updateXList(rows[j]->getStartCoor().x, rows[j]->getEndX(), xList);
            }
            if((*xList.begin()).startX <= startX && (*xList.begin()).endX >= endX){
                siteHeight = rows[j]->getStartCoor().y - rows[i]->getStartCoor().y;
                break;
            }
        }
        rows[i]->setSiteHeight(siteHeight);
        
    }
    // DEBUG
    // for(size_t i = 0; i < rows.size(); i++){
    //     Row* row = rows[i];
    //     vector<Subrow*> subrows = row->getSubrows();
    //     std::cout << row-> getSiteHeight() << std::endl;
    //     for(size_t j = 0; j < subrows.size(); j++){
    //         std::cout << subrows[j]-> getStartX() << " " << subrows[j]-> getEndX() << " " << subrows[j]-> getHeight() << std::endl;
    //     }
    // }
    // DEBUG
    // std::list<XTour> xList;
    // updateXList(2, 4, xList);
    // updateXList(6, 12, xList);
    // updateXList(13, 16, xList);
    // updateXList(2, 19, xList);
    // updateXList(5, 8, xList);
    // for(XTour xtour : xList){
    //     std::cout << xtour.startX << " " << xtour.endX << std::endl;
    // }



}

void Legalizer::updateXList(double start, double end, std::list<XTour> & xList){
    if(xList.empty()){
        XTour insertRow;
        insertRow.startX = start;
        insertRow.endX = end;
        xList.push_back(insertRow);
        return;
    }

    auto iter1 = xList.begin();
    bool startIsFound = false;
    while(iter1 != xList.end() && start >= (*iter1).startX){
        if(start >= (*iter1).startX && start <= (*iter1).endX){
            startIsFound = true;
            break;
        } 
        iter1++;
    }
    auto iter2 = iter1;
    bool endIsFound = false;
    while(iter2 != xList.end() && end >= (*iter2).startX){
        if(end >= (*iter2).startX && end <= (*iter2).endX){
            endIsFound = true;
            break;
        }
        iter2++;
    }
    // std::cout << "iter1:"<< (*iter1).startX << " " << (*iter1).endX << std::endl;
    // std::cout << "iter2:"<< (*iter2).startX << " " << (*iter2).endX << std::endl;
    if(!startIsFound && !endIsFound){
        std::cout << "All Not Found!" << std::endl;
        XTour insertRow = {start, end};
        if(iter1 != iter2){
            auto iter = xList.erase(iter1, iter2);
            xList.insert(iter, insertRow);
        }else{
            xList.insert(iter1, insertRow);
        }
    }else{
        if(iter1 == iter2){
            (*iter1).endX = ((*iter1).endX >= end) ? (*iter1).endX : end;
        }else{
            XTour insertRow = {start, end};
            std::list<XTour>::iterator eraseEnd;
            if(endIsFound){
                eraseEnd = next(iter2);
            }else{
                eraseEnd = iter2;
            }
            auto iter = xList.erase(iter1, eraseEnd);
            xList.insert(iter, insertRow);
        }
    }

}

void Legalizer::SliceRowsByGate(){
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
    // Sort cell by cost function, consider timing in first order?
    std::sort(ffs.begin(), ffs.end(), [](Node *a, Node *b){
        double costA = a->getH() * a->getW();
        double costB = b->getH() * b->getW();
        return costA > costB;
    });
    

    DEBUG_LGZ("Start Legalize FF");
    int closest_row_idx = 0;
    int down_row_idx = 0;
    int up_row_idx = 0;

    for(const auto &ff : ffs){
        closest_row_idx = FindClosestRow(ff);
        double minDisplacement = PlaceMultiHeightFFOnRow(ff, closest_row_idx);
        double localMinDisplacementDown = std::numeric_limits<double>::max();
        double localMinDisplacementUp = std::numeric_limits<double>::max();

        // [TODO, need to return the best coor from up and down, and assgin the coordinate from the better one]
        #pragma omp parallel
        {
            #pragma omp sections
            {
                #pragma omp section
                {
                    // search down
                    down_row_idx = closest_row_idx - 1;
                    while(down_row_idx >= 0 && std::abs(ff->getGPCoor().y - rows[down_row_idx]->getStartCoor().y) < localMinDisplacementDown){
                        double downDisplacement = PlaceMultiHeightFFOnRow(ff, down_row_idx);
                        localMinDisplacementDown = localMinDisplacementDown < downDisplacement ? localMinDisplacementDown : downDisplacement;
                        down_row_idx--;
                    }
                }

                #pragma omp section
                {
                    // search up
                    up_row_idx = closest_row_idx + 1;
                    while(up_row_idx < (int)rows.size() && std::abs(ff->getGPCoor().y - rows[up_row_idx]->getStartCoor().y) < localMinDisplacementUp){
                        double upDisplacement = PlaceMultiHeightFFOnRow(ff, up_row_idx);
                        localMinDisplacementUp = localMinDisplacementUp < upDisplacement ? localMinDisplacementUp : upDisplacement;
                        up_row_idx++;
                    }
                }
            }
        }


        if(ff->getIsPlace()){
            for(auto &row : rows){
                row->slicing(ff);
            }
        }

        // change cell type, iterate throught the cell library, consider area and aspect ratio
        if((!ff->getIsPlace()) && ENABLE_CHANGE_CELL_TYPE){
            int numBits = mgr.FF_Map[ff->getName()]->getCell()->getBits();
            for(size_t i = 0; i < mgr.Bit_FF_Map[numBits].size(); i++){
                mgr.FF_Map[ff->getName()]->setCell(mgr.Bit_FF_Map[numBits][i]);
                ff->setW(mgr.Bit_FF_Map[numBits][i]->getW());
                ff->setH(mgr.Bit_FF_Map[numBits][i]->getH());

                closest_row_idx = FindClosestRow(ff);
                double minDisplacement = PlaceMultiHeightFFOnRow(ff, closest_row_idx);
                double localMinDisplacementDown = std::numeric_limits<double>::max();
                double localMinDisplacementUp = std::numeric_limits<double>::max();

                // [TODO, need to return the best coor from up and down, and assgin the coordinate from the better one]
                #pragma omp parallel
                {
                    #pragma omp sections
                    {
                        #pragma omp section
                        {
                            // search down
                            down_row_idx = closest_row_idx - 1;
                            while(down_row_idx >= 0 && std::abs(ff->getGPCoor().y - rows[down_row_idx]->getStartCoor().y) < localMinDisplacementDown){
                                double downDisplacement = PlaceMultiHeightFFOnRow(ff, down_row_idx);
                                localMinDisplacementDown = localMinDisplacementDown < downDisplacement ? localMinDisplacementDown : downDisplacement;
                                down_row_idx--;
                            }
                        }

                        #pragma omp section
                        {
                            // search up
                            up_row_idx = closest_row_idx + 1;
                            while(up_row_idx < (int)rows.size() && std::abs(ff->getGPCoor().y - rows[up_row_idx]->getStartCoor().y) < localMinDisplacementUp){
                                double upDisplacement = PlaceMultiHeightFFOnRow(ff, up_row_idx);
                                localMinDisplacementUp = localMinDisplacementUp < upDisplacement ? localMinDisplacementUp : upDisplacement;
                                up_row_idx++;
                            }
                        }
                    }
                }
                if(ff->getIsPlace()){
                    DEBUG_LGZ("Change Type LGZ success");
                    break;
                } 
            }
        }

        if(ff->getIsPlace()){
            for(auto &row : rows){
                row->slicing(ff);
            }
        }


        if(!ff->getIsPlace()){
            // Global search mode
            DEBUG_LGZ("Legalize " + ff->getName() + " FF Failed => Enter Greedy Global Search Mode");
            localMinDisplacementDown = std::numeric_limits<double>::max();
            localMinDisplacementUp = std::numeric_limits<double>::max();
            #pragma omp parallel
            {
                #pragma omp sections
                {
                    #pragma omp section
                    {
                        // search down
                        down_row_idx = closest_row_idx - 1;
                        while(down_row_idx >= 0){
                            double downDisplacement = PlaceMultiHeightFFOnRow(ff, down_row_idx);
                            localMinDisplacementDown = localMinDisplacementDown < downDisplacement ? localMinDisplacementDown : downDisplacement;
                            down_row_idx--;
                            if(ff->getIsPlace()) break;
                        }
                    }

                    #pragma omp section
                    {
                        // search up
                        up_row_idx = closest_row_idx + 1;
                        while(up_row_idx < (int)rows.size()){
                            double upDisplacement = PlaceMultiHeightFFOnRow(ff, up_row_idx);
                            localMinDisplacementUp = localMinDisplacementUp < upDisplacement ? localMinDisplacementUp : upDisplacement;
                            up_row_idx++;
                            if(ff->getIsPlace()) break;
                        }
                    }
                }
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

// find a closest subrow index to trial place
int Legalizer::FindClosestSubrow(Row *row, Node *ff){
    double startX = ff->getGPCoor().x;
    std::vector<Subrow *> subrows = row->getSubrows();
    for(size_t i = 0; i < subrows.size(); i++){
        if(subrows[i]->getStartX() > startX){
            return (i - 1) >= 0 ? (i - 1): 0;
        }
    }
    return 0;
}


// Try to place on this row, for multi-row height, must check upper row can place or not
// Return the displacement from the global placement coordinate
// double Legalizer::PlaceMultiHeightFFOnRow(Node *ff, int row_idx) {
//     double minDisplacement = ff->getDisplacement(ff->getLGCoor());
//     const auto &subrows = rows[row_idx]->getSubrows();
//     Coor bestCoor = ff->getLGCoor();        // Track of the best coordinate
//     bool ffIsPlace = false;               // Track of whether a placement was made

//     for(size_t i = 0; i < subrows.size(); i++){
//         const auto &subrow = subrows[i];
//         double alignedStartX = rows[row_idx]->getStartCoor().x + std::ceil((int)(subrow->getStartX() - rows[row_idx]->getStartCoor().x) / rows[row_idx]->getSiteWidth()) * rows[row_idx]->getSiteWidth();
//         for(int x = alignedStartX; x <= subrow->getEndX(); x += rows[row_idx]->getSiteWidth()){
//             bool placeable = ContinousAndEmpty(x, rows[row_idx]->getStartCoor().y, ff->getW(), ff->getH(), row_idx);
//             Coor currCoor = Coor(x, rows[row_idx]->getStartCoor().y);
//             double displacement = ff->getDisplacement(currCoor);
//             if (placeable && displacement < minDisplacement){
//                 minDisplacement = displacement;
//                 bestCoor = currCoor;
//                 ffIsPlace = true;
//             }
//         }
//     }
//     if (ffIsPlace) {
//         ff->setLGCoor(bestCoor);
//         ff->setIsPlace(true);
//     }
//     return minDisplacement;
// }

// Need claire to approve this method...
double Legalizer::PlaceMultiHeightFFOnRow(Node *ff, int row_idx){
    double minDisplacement = ff->getDisplacement(ff->getLGCoor());
    const auto &subrows = rows[row_idx]->getSubrows();
    Coor bestCoor = ff->getLGCoor();        // Track of the best coordinate
    bool ffIsPlace = false;               // Track of whether a placement was made
    int closestSubrowIdx = FindClosestSubrow(rows[row_idx], ff);  // Find a good entry(subrow index) for this row

    // bisect the row to find the optimal location.
    // 1. Search right
    bool finishSearchRight = false;
    for(size_t i = closestSubrowIdx; i < subrows.size(); i++){
        const auto &subrow = subrows[i];
        double alignedStartX = rows[row_idx]->getStartCoor().x + std::ceil((int)(subrow->getStartX() - rows[row_idx]->getStartCoor().x) / rows[row_idx]->getSiteWidth()) * rows[row_idx]->getSiteWidth();
        // iterate through all on site points
        for(int x = alignedStartX; x <= subrow->getEndX(); x += rows[row_idx]->getSiteWidth()){
            bool foundBetter = false;
            bool placeable = ContinousAndEmpty(x, rows[row_idx]->getStartCoor().y, ff->getW(), ff->getH(), row_idx);
            Coor currCoor = Coor(x, rows[row_idx]->getStartCoor().y);
            double displacement = ff->getDisplacement(currCoor);
            if (placeable && displacement < minDisplacement){
                minDisplacement = displacement;
                bestCoor = currCoor;
                ffIsPlace = true;
                foundBetter = true;
            }
            // Will always found worse answer => stop search right
            if(ffIsPlace && placeable && !foundBetter){
                finishSearchRight = true;
                break;
            }
        }
        if(finishSearchRight) break;
    }

    // 2. Search left
    bool finishSearchLeft = false;
    for(int i = closestSubrowIdx - 1; i >= 0; i--){
        const auto &subrow = subrows[i];
        double alignedStartX = rows[row_idx]->getStartCoor().x + std::floor((int)(subrow->getEndX() - rows[row_idx]->getStartCoor().x) / rows[row_idx]->getSiteWidth()) * rows[row_idx]->getSiteWidth();
        // iterate through all on site points
        for(int x = alignedStartX; x >= subrow->getStartX(); x -= rows[row_idx]->getSiteWidth()){
            bool foundBetter = false;
            bool placeable = ContinousAndEmpty(x, rows[row_idx]->getStartCoor().y, ff->getW(), ff->getH(), row_idx);
            Coor currCoor = Coor(x, rows[row_idx]->getStartCoor().y);
            double displacement = ff->getDisplacement(currCoor);
            if (placeable && displacement < minDisplacement){
                minDisplacement = displacement;
                bestCoor = currCoor;
                ffIsPlace = true;
                foundBetter = true;
            }
            // Will always found worse answer => stop search left
            if(ffIsPlace && placeable && !foundBetter && displacement > minDisplacement){
                finishSearchLeft = true;
                break;
            }
        }
        if(finishSearchLeft) break;
    }
    if (ffIsPlace) {
        ff->setLGCoor(bestCoor);
        ff->setIsPlace(true);
    }
    return minDisplacement;
}