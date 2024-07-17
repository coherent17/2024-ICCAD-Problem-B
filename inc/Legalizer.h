#ifndef _LEGALIZER_H_
#define _LEGALIZER_H_

#include "Node.h"
#include "Row.h"
#include "Subrow.h"
#include "Manager.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cfloat>

#define DEBUG_LGZ(message) std::cout << "[LEGALIZER] " << message << std::endl
#define AREA_WEIGHT 1000
#define HEIGHT_WEIGHT 2000
#define WIDTH_WEIGHT 2000
#define X_WEIGHT 100
#define SEARCH_FAILURE -1

class Legalizer{
private:
    Manager& mgr;
    int numffs;
    int numgates;
    int numrows;
    std::vector<Node *> ffs;
    std::vector<Node *> gates;
    std::vector<Row *> rows;
    double minRowHeight;

public:
    Legalizer(Manager& mgr);
    ~Legalizer();

    bool run();

private:
    void ConstructDB();
    void LoadFF();
    void LoadGate();
    void LoadPlacementRow();
    void SliceRows();
    void Abacus();
    void LegalizeResultWriteBack();

    // helper function
    bool IsOverlap(const Coor &coor1, double w1, double h1, const Coor &coor2, double w2, double h2);
    bool ContinousAndEmpty(double startX, double startY, double w, double h, int row_idx);
    void CheckSubrowsAttribute();
    int FindClosestRow(Node *ff);
    double PlaceMultiHeightFFOnRow(Node *ff, int row_idx);
};

#endif