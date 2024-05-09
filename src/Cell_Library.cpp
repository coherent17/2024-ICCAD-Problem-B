#include "Cell_Library.h"

Cell_Library::Cell_Library(){
    ;
}

Cell_Library::~Cell_Library(){
    ;
}

void Cell_Library::addCell(const string &cellName, const Cell &c){
    Cell_Map[cellName] = c;
}

ostream &operator<<(ostream &out, const Cell_Library &c){
    for(const auto &pair : c.Cell_Map){
        out << pair.first << " " << pair.second << endl;
    }
    return out;
}