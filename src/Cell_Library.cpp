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

Cell *Cell_Library::getCell(const string &cellName){
    return &Cell_Map[cellName];
}

bool Cell_Library::isFF(const string &cellName){
    return Cell_Map[cellName].getisFF();
}

ostream &operator<<(ostream &out, const Cell_Library &c){
    for(const auto &pair : c.Cell_Map){
        out << pair.second << endl;
    }
    return out;
}