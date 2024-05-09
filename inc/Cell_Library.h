#ifndef _CELL_LIBRARY_H_
#define _CELL_LIBRARY_H_

#include "Cell.h"

class Cell_Library{
private:
    unordered_map<string, Cell> Cell_Map;

public:
    Cell_Library();
    ~Cell_Library();
    void addCell(const string &, const Cell &);

    friend ostream &operator<<(ostream &, const Cell_Library &);
};

#endif