#include "Legalizer.h"
#include <thread>
#include <mutex>

Legalizer::Legalizer(Manager& mgr) : mgr(mgr){
    // timer.start();
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

void Legalizer::initial(){
    // LoadFF();
    LoadGate();
    LoadPlacementRow();
    SliceRowsByRows();
    SliceRowsByGate();
}

void Legalizer::run(){
    // CheckIfMBFFMove();
    LoadFF();
    Tetris();
    LegalizeWriteBack();
    // timer.stop();
}

void Legalizer::LoadFF(){
    DEBUG_LGZ("Load FF to Databse");
    for(const auto &pair : mgr.FF_Map){
        if(!pair.second->getIsLegalize()){
            Node *ff = new Node();
            ff->setName(pair.second->getInstanceName());
            ff->setGPCoor(pair.second->getNewCoor());
            ff->setLGCoor(Coor(DBL_MAX, DBL_MAX));
            ff->setCell(pair.second->getCell());
            ff->setW(pair.second->getW());
            ff->setH(pair.second->getH());
            ff->setIsPlace(false);
            ff->setTNS(pair.second->getTNS());
            ff->setFFPtr(pair.second);
            ffs.emplace_back(ff);
        }
    }
}

//todo: renew Rows if FF position change and LG again
// void Legalizer::CheckIfMBFFMove(){
//     for(size_t i = 0; i < ffs.size(); i++){
//         Node* ff = ffs[i];
//         string FFName = ff->getName();
//         FF* ffptrInMap = mgr.FF_Map[FFName];
//         if(ff->getLGCoor().x != ffptrInMap->getNewCoor().x || ff->getLGCoor().y != ffptrInMap->getNewCoor().y){
//             ff->setIsPlace(false);
//             ff->setGPCoor(ffptrInMap->getNewCoor());
//             ff->setLGCoor(Coor(DBL_MAX, DBL_MAX));
//             ff->setTNS(ffptrInMap->getTNS());
//         }
//     }
// }

void Legalizer::LoadGate(){
    DEBUG_LGZ("Load Gate to Databse");
    for(const auto &pair: mgr.Gate_Map){
        Node *gate = new Node();
        gate->setName(pair.second->getInstanceName());
        gate->setGPCoor(pair.second->getCoor());
        gate->setLGCoor(pair.second->getCoor());
        gate->setCell(nullptr);
        gate->setW(pair.second->getW());
        gate->setH(pair.second->getH());
        gate->setIsPlace(false);
        gates.emplace_back(gate);
    }
}

void Legalizer::LoadPlacementRow(){
    DEBUG_LGZ("Load Placement Row to Databse");
    std::vector<PlacementRow> PlacementRows = mgr.die.getPlacementRows();
    for(size_t i = 0; i < PlacementRows.size(); i++){
        Row *row = new Row();
        row->setStartCoor(PlacementRows[i].startCoor);
        row->setSiteHeight(mgr.die.getDieBorder().y - PlacementRows[i].startCoor.y);
        row->setSiteWidth(PlacementRows[i].siteWidth);
        row->setNumOfSite(PlacementRows[i].NumOfSites);
        row->setEndX(PlacementRows[i].startCoor.x + PlacementRows[i].siteWidth * PlacementRows[i].NumOfSites);

        // init Row::subrows
        Subrow *subrow = new Subrow();
        subrow->setStartX(PlacementRows[i].startCoor.x);
        subrow->setEndX(PlacementRows[i].startCoor.x + PlacementRows[i].siteWidth * PlacementRows[i].NumOfSites);
        subrow->setFreeWidth(PlacementRows[i].siteWidth * PlacementRows[i].NumOfSites);
        subrow->setHeight(mgr.die.getDieBorder().y - PlacementRows[i].startCoor.y);
        row->addSubrows(subrow);
        rows.emplace_back(row);
    }

    // sort row by the y coordinate in ascending order, if tie, sort by x in ascending order
    std::sort(rows.begin(), rows.end(), [](const Row *a, const Row *b){
        return *a < *b;
    });
}


void Legalizer::SliceRowsByRows(){
    for(size_t i = 0; i < rows.size() - 1; i++){
        double startX = rows[i]->getStartCoor().x;
        double endX = rows[i]->getEndX();
        double siteHeight = rows[i]->getSiteHeight();
        std::list<XTour> xList;
        for(size_t j = i + 1; j < rows.size(); j++){
            if(rows[j]->getEndX() <= startX || rows[j]->getStartCoor().x >= endX) continue;
            Node * upRow = new Node();
            upRow->setGPCoor(rows[j]->getStartCoor());
            upRow->setLGCoor(rows[j]->getStartCoor());
            upRow->setW(rows[j]->getEndX() - rows[j]->getStartCoor().x);
            upRow->setH(rows[j]->getSiteHeight());
            rows[i]->slicing(upRow);
            UpdateXList(rows[j]->getStartCoor().x, rows[j]->getEndX(), xList);

            if((*xList.begin()).startX <= startX && (*xList.begin()).endX >= endX){
                siteHeight = rows[j]->getStartCoor().y - rows[i]->getStartCoor().y;
                break;
            }
        }
        rows[i]->setSiteHeight(siteHeight);
    }
}

void Legalizer::SliceRowsByGate(){
    DEBUG_LGZ("Seperate PlacementRows by Gate Cell");
    for(const auto &gate : gates){
        for(auto &row : rows){
            if(row->getStartCoor().y > gate->getGPCoor().y + gate->getH()) break;
            row->slicing(gate);
        }
        gate->setIsPlace(true);
    }
}

Coor Legalizer::FindPlace(const Coor &coor, Cell * cell){
    size_t closest_row_idx = FindClosestRow(coor);
    double minDisplacement = DBL_MAX;
    Coor newCoor = Coor(DBL_MAX, DBL_MAX);

    bool placeable = true;
    PredictFFLGPlace(coor, cell, closest_row_idx, placeable, minDisplacement, newCoor);
    int down_row_idx = closest_row_idx - 1;
    int up_row_idx = closest_row_idx + 1;
    // local search down
    while(down_row_idx >= 0 && std::abs(coor.y - rows[down_row_idx]->getStartCoor().y) < minDisplacement){
        if(!rows[down_row_idx]->hasCell(cell)){
            placeable = true;
            PredictFFLGPlace(coor, cell, down_row_idx, placeable, minDisplacement, newCoor);
            if(!placeable)
                rows[down_row_idx]->addRejectCell(cell);
        }
        down_row_idx--;
    }
    // local search up
    while(up_row_idx < (int)rows.size() && std::abs(coor.y - rows[up_row_idx]->getStartCoor().y) < minDisplacement){
        if(!rows[up_row_idx]->hasCell(cell)){
            placeable = true;
            PredictFFLGPlace(coor, cell, up_row_idx, placeable, minDisplacement, newCoor);
            if(!placeable)
                rows[up_row_idx]->addRejectCell(cell);
        }
        up_row_idx++;
    }
    return newCoor;
}

void Legalizer::UpdateRows(FF* newFF){
    Node *ff = new Node();
    ff->setName(newFF->getInstanceName());
    ff->setGPCoor(newFF->getNewCoor());
    ff->setLGCoor(newFF->getNewCoor());
    ff->setCell(newFF->getCell());
    ff->setW(newFF->getW());
    ff->setH(newFF->getH());
    ff->setIsPlace(true);
    ff->setTNS(newFF->getTNS());
    ff->setFFPtr(newFF);
    ffs.emplace_back(ff);
    for(size_t i = 0; i < rows.size(); i++){
        Row *row = rows[i];
        if(row->getStartCoor().y > ff->getLGCoor().y + ff->getH()) break;
        row->slicing(ff);
        if(row->getStartCoor().y == ff->getLGCoor().y && row->getStartCoor().x <= ff->getLGCoor().x && row->getEndX() > ff->getLGCoor().x){
            ff->setPlaceRowIdx(i);// for DP
        }
            
    }
}

void Legalizer::Tetris(){
    DEBUG_LGZ("Start Legalize FF");
    std::sort(ffs.begin(), ffs.end(), [](const Node *a, const Node *b){
        double costA = a->getH() * a->getW();
        double costB = b->getH() * b->getW();
        if(costA != costB)
            return costA > costB;
        else
            return a->getTNS() > b->getTNS();
    });

    // start to legalize all ff
    size_t total = ffs.size();
    size_t quarter = ffs.size() / 4; 
    for(size_t i = 0; i < ffs.size(); i++){
        if(i == quarter || i == 2 * quarter || i == 3 * quarter || i == total - 1) {
            DEBUG_LGZ("Progress: " + std::to_string((i + 1) * 100 / total) + "% completed");
        }
        
        if(ffs[i]->getIsPlace()) continue;
        Node *ff = ffs[i];
        int numBits = ff->getCell()->getBits();
        int cell_idx = 0;        
        size_t closest_row_idx = FindClosestRow(ff->getGPCoor());
        while(1){
            bool placeable = true;
            double minDisplacement = PlaceFF(ff, closest_row_idx, placeable);
            int down_row_idx = closest_row_idx - 1;
            int up_row_idx = closest_row_idx + 1;
            // local search down
            while(down_row_idx >= 0 && std::abs(ff->getGPCoor().y - rows[down_row_idx]->getStartCoor().y) < minDisplacement){
                if(!rows[down_row_idx]->hasCell(ff->getCell())){
                    placeable = true;
                    double downDisplacement = PlaceFF(ff, down_row_idx, placeable);
                    if(placeable)
                        minDisplacement = minDisplacement < downDisplacement ? minDisplacement : downDisplacement;
                    else
                        rows[down_row_idx]->addRejectCell(ff->getCell());
                }
                down_row_idx--;
            }

            // local search up
            while(up_row_idx < (int)rows.size() && std::abs(ff->getGPCoor().y - rows[up_row_idx]->getStartCoor().y) < minDisplacement){
                if(!rows[up_row_idx]->hasCell(ff->getCell())){
                    placeable = true;
                    double upDisplacement = PlaceFF(ff, up_row_idx, placeable);
                    if(placeable)
                        minDisplacement = minDisplacement < upDisplacement ? minDisplacement : upDisplacement;
                    else
                        rows[up_row_idx]->addRejectCell(ff->getCell());
                }
                up_row_idx++;
            }

            if(ff->getIsPlace()){
                for(auto &row : rows){
                    if(row->getStartCoor().y > ff->getLGCoor().y + ff->getH()) break;
                    row->slicing(ff);
                }
                break;
            }
            else{
                cell_idx++;
                // change cell type (in top 3) and local search
                if(cell_idx >= (int)mgr.Bit_FF_Map[numBits].size()){
                    DEBUG_LGZ("Legalized Fail");
                    break;
                }
                DEBUG_LGZ("Change Cell Type");
                mgr.FF_Map[ff->getName()]->setCell(mgr.Bit_FF_Map[numBits][cell_idx]);
                ff->setCell(mgr.Bit_FF_Map[numBits][cell_idx]);
                ff->setW(mgr.Bit_FF_Map[numBits][cell_idx]->getW());
                ff->setH(mgr.Bit_FF_Map[numBits][cell_idx]->getH());
            }

        }
    }
}

void Legalizer::LegalizeWriteBack(){
    DEBUG_LGZ("Write Back Legalize Coordinate");
    for(const auto &ff : ffs){
        if(ff->getIsPlace()){
            mgr.FF_Map[ff->getName()]->setNewCoor(ff->getLGCoor());
            //std::cout << *ff << std::endl;
        }
        else{
            mgr.FF_Map[ff->getName()]->setNewCoor(Coor(0, 0));
        }
    }
}

// Helper Function
void Legalizer::UpdateXList(double start, double end, std::list<XTour> & xList){
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
    if(!startIsFound && !endIsFound){
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

// size_t Legalizer::FindClosestRow(Node *ff){
//     double min_distance = std::abs(ff->getGPCoor().y - rows[0]->getStartCoor().y);
//     for(size_t i = 1; i < rows.size(); i++){
//         double curr_distance = std::abs(ff->getGPCoor().y - rows[i]->getStartCoor().y);
//         if(curr_distance < min_distance){
//             min_distance = curr_distance;
//         }
//         else{
//             return i - 1;
//         }
//     }
//     return rows.size() - 1;
// }

size_t Legalizer::FindClosestRow(const Coor &coor) {
    double targetY = coor.y;
    size_t left = 0;
    size_t right = rows.size() - 1;

    while (left < right) {
        size_t mid = left + (right - left) / 2;
        double midY = rows[mid]->getStartCoor().y;

        if (midY == targetY) {
            return mid;
        } else if (midY < targetY) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }

    // Check the nearest row among the final candidates
    if (left > 0 && std::abs(targetY - rows[left]->getStartCoor().y) >= std::abs(targetY - rows[left - 1]->getStartCoor().y)) {
        return left - 1;
    }

    return left;
}



int Legalizer::FindClosestSubrow(Node *ff, Row *row){
    const auto &subrows = row->getSubrows();
    assert(subrows.size() > 0);

    double startX = ff->getGPCoor().x;
    for(int i = 0; i < (int)subrows.size(); i++){

        if(subrows[i]->getStartX() > startX){
            return (i - 1) >= 0 ? (i - 1): 0;
        }
    }
    return subrows.size() - 1;
}


void Legalizer::PredictFFLGPlace(const Coor &coor, Cell* cell, size_t row_idx, bool &placeable, double &minDisplacement, Coor &newCoor){
    const auto &subrows = rows[row_idx]->getSubrows();
    bool skip = false;
    bool rowCanPlace = false;

    for(size_t i = 0; i < subrows.size(); i++){
        const auto &subrow = subrows[i];
        if(subrow->hasCell(cell)) continue;
        double alignedStartX = rows[row_idx]->getStartCoor().x + std::ceil((int)(subrow->getStartX() - rows[row_idx]->getStartCoor().x) / rows[row_idx]->getSiteWidth()) * rows[row_idx]->getSiteWidth(); 
        bool subrowSkip = false;
        bool subrowCanPlace = false;
        for(int x = alignedStartX; x <= subrow->getEndX() && x != rows[row_idx]->getEndX(); x += rows[row_idx]->getSiteWidth()){
            Coor currCoor = Coor(x, rows[row_idx]->getStartCoor().y);
            if(getDisplacement(coor, currCoor) > minDisplacement){
                subrowSkip = true;
                skip = true;
                Coor subrowEndCoor = Coor(subrow->getEndX(), rows[row_idx]->getStartCoor().y);
                // If current subrow can't find better solution
                if(getDisplacement(coor, subrowEndCoor) > minDisplacement) break;
                continue;
            }
            bool canPlace = ContinousAndEmpty(x, rows[row_idx]->getStartCoor().y, cell->getW(), cell->getH(), row_idx);
            double displacement = getDisplacement(coor, currCoor);

            if(canPlace){
                rowCanPlace = true;
                subrowCanPlace = true;
                if(displacement < minDisplacement){
                    minDisplacement = displacement;
                    newCoor = currCoor;
                }
            }
        }
        if(!subrowCanPlace && !subrowSkip)
            subrow->addRejectCell(cell);

    }
    if(!skip && !rowCanPlace){
        placeable = false;
    }
}


double Legalizer::PlaceFF(Node *ff, size_t row_idx, bool &placeable){
    double minDisplacement = ff->getDisplacement();
    const auto &subrows = rows[row_idx]->getSubrows();
    bool skip = false;
    bool rowCanPlace = false;

    for(size_t i = 0; i < subrows.size(); i++){
        const auto &subrow = subrows[i];
        if(subrow->hasCell(ff->getCell())) continue;
        double alignedStartX = rows[row_idx]->getStartCoor().x + std::ceil((int)(subrow->getStartX() - rows[row_idx]->getStartCoor().x) / rows[row_idx]->getSiteWidth()) * rows[row_idx]->getSiteWidth(); 
        bool subrowSkip = false;
        bool subrowCanPlace = false;
        for(int x = alignedStartX; x <= subrow->getEndX() && x != rows[row_idx]->getEndX(); x += rows[row_idx]->getSiteWidth()){
            Coor currCoor = Coor(x, rows[row_idx]->getStartCoor().y);
            if(ff->getDisplacement(currCoor) > minDisplacement){
                subrowSkip = true;
                skip = true;
                Coor subrowEndCoor = Coor(subrow->getEndX(), rows[row_idx]->getStartCoor().y);
                // If current subrow can't find better solution
                if(ff->getDisplacement(subrowEndCoor) > minDisplacement) break;
                continue;
            }
            bool canPlace = ContinousAndEmpty(x, rows[row_idx]->getStartCoor().y, ff->getW(), ff->getH(), row_idx);
            double displacement = ff->getDisplacement(currCoor);
                
            if(canPlace){
                rowCanPlace = true;
                subrowCanPlace = true;
                if(displacement < minDisplacement){
                    minDisplacement = displacement;
                    ff->setLGCoor(currCoor);
                    ff->setIsPlace(true);
                    ff->setPlaceRowIdx(row_idx);
                }
            }
        }
        if(!subrowCanPlace && !subrowSkip)
            subrow->addRejectCell(ff->getCell());

    }
    if(!skip && !rowCanPlace){
        placeable = false;
    }
    return minDisplacement;
}


double Legalizer::getDisplacement(const Coor &Coor1, const Coor &Coor2){
    return std::sqrt(std::pow(Coor1.x - Coor2.x, 2) + std::pow(Coor1.y - Coor2.y, 2));
} 
    

bool Legalizer::ContinousAndEmpty(double startX, double startY, double w, double h, int row_idx){
    double endY = startY + h;
    double currentY = startY;

    for(size_t i = row_idx; i < rows.size(); i++){
        double rowStartY = rows[i]->getStartCoor().y;
        double rowStartX = rows[i]->getStartCoor().x;
        double rowEndX = rowStartX + rows[i]->getSiteWidth() * rows[i]->getNumOfSite();

        // check if the current row is upper than currentY -> early break
        if(rowStartY > currentY) break;
        // check if the current row is target row -> jump condition
        if(rowStartY != currentY || startX < rows[i]->getStartCoor().x || startX + w > rowEndX) continue;
        // Save highest space can place in range startX to startX + w
        double placeH = DBL_MAX;
        // check if this row is continuous from startX to startX + w
        if(rows[i]->canPlace(startX, startX + w, placeH)){
            // update current continuous y
            currentY += placeH;

            // Check if we reached the target height
            if(currentY >= endY) return true;
        }
    }
    return false;
}
