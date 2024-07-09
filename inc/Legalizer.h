#ifndef _LEGALIZER_H_
#define _LEGALIZER_H_

#include "Node.h"
#include "Row.h"
#include "Subrow.h"
#include "Manager.h"
#include <vector>
#include <string>
#include <algorithm>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>

#define DEBUG_LGZ(message) std::cout << "[LEGALIZER] " << message << std::endl

class Legalizer{
private:
    Manager& mgr;
    int numffs;
    int numgates;
    int numrows;
    std::vector<Node *> ffs;
    std::vector<Node *> gates;
    std::vector<Row *> rows;

public:
    Legalizer(Manager& mgr);
    ~Legalizer();

    void run();

private:
    void ConstructDB();
    void LoadFF();
    void LoadGate();
    void LoadPlacementRow();
    void SliceRows();

    // helper function
    bool IsOverlap(const Coor &coor1, double w1, double h1, const Coor &coor2, double w2, double h2);

    // unit test:
    //Testcase for testing Row::slicing()
    void row_slicing_unit_test();
};

#endif