#ifndef _FAST_LEGALIZER_H_
#define _FAST_LEGALIZER_H_

#include "Cell.h"
#include "Node.h"
#include "Row.h"
#include "Subrow.h"
#include "Manager.h"
#include "Timer.h"
#include "Util.h"
#include "XTour.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cfloat>

#define DEBUG_FASTLGZ(message) std::cout << "[FAST_LEGALIZER] " << message << std::endl

class Fast_Legalizer{
private:
    Manager& mgr;
    std::vector<Node *> ffs;
    std::vector<Node *> gates;
    std::vector<Row *> rows;
    std::vector<Node *> fail_ffs;
    Timer timer;

public:
    explicit Fast_Legalizer(Manager& mgr);
    ~Fast_Legalizer();
    void run();

private:
    void LoadFF();
    void LoadGate();
    void LoadPlacementRow();
    void SliceRowsByRows();
    void SliceRowsByGate();
    void Tetris();
    void LegalizeWriteBack();
    

    // Helper Function
    void UpdateXList(double start, double end, std::list<XTour> & xList);
    size_t FindClosestRow(Node *ff);
    int FindClosestSubrow(Node *ff, Row *row);
    double PlaceFF(Node *ff, size_t row_idx, Coor &bestCoor);
    bool ContinousAndEmpty(double startX, double startY, double w, double h, int row_idx);
    
};

#endif